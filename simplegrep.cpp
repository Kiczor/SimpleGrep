#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <string>
#include <regex>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include "ThreadPool.h"

std::string logfilepath, resultfilepath, dirpath, pattern;
int threadnumber;

//global stats
int searched_files, files_with_pattern;
timeval start_time;

typedef std::pair<std::string, std::pair<int, std::string> > fileline;

std::vector<fileline> found;

std::map<std::thread::id, std::vector<std::string> > threadlogs;

std::mutex foundmutex;

void SeekInFile( std::string path, std::string pattern )
{
    std::regex patternregex( pattern );

    std::ifstream filestream(path);
    std::string line;

    int count = 0;
    while( getline( filestream, line ) )
    {
        count++;
        if( std::regex_search( line, patternregex ) )
        {
            std::unique_lock<std::mutex> lock(foundmutex);
            found.push_back( make_pair( path, make_pair( count, line ) ) );
        }
    }

    return;
}

int main(int argc, char** argv)
{
    gettimeofday(&start_time, 0);

    dirpath = ".";
    logfilepath = "logfile.txt";
    resultfilepath = "resultfile.txt";
    threadnumber = 4;

    int c;
    while( 1 ) // handling options
    {
        static struct option long_options[] =
        {
            {"dir", required_argument, 0, 'd'},
            {"log_file", required_argument, 0, 'l'},
            {"result_file", required_argument, 0, 'r'},
            {"threads", required_argument, 0, 't'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long(argc, argv, "d:l:r:t:", long_options, &option_index);

        if( c == -1 )
            break;

        switch( c )
        {
            case 'd':
                dirpath = optarg;
                break;
            case 'l':
                logfilepath = optarg;
                break;
            case 'r':
                resultfilepath = optarg;
                break;
            case 't':
                threadnumber = atoi( optarg );
                break;
            case '?':
                break;
        }
    }

    pattern = argv[ argc - 1 ];

    const std::filesystem::path fspath = dirpath;

    ThreadPool* mythreadpool = new ThreadPool(threadnumber);
    for (const auto& direntry : std::filesystem::recursive_directory_iterator(fspath))
    {
        mythreadpool -> AddWork( [direntry] { SeekInFile( direntry.path(), pattern ); }, direntry.path().filename() );
        searched_files += 1;
    }
    while( mythreadpool -> busy() ) {}
    threadlogs = mythreadpool -> filelogs;
    delete mythreadpool;

    sort( found.begin(), found.end() );

    //counting different files with pattern
    std::vector< std::pair< int, fileline > > linesbycount;

    int currentcount = 0, firstfound = 0;
    std::string currentpath = "";
    if( found.size() > 0 )
    {
        currentpath = found[0].first;
        currentcount = 1;
        firstfound = 0;
        for(int i = 1; i < (int)found.size(); i++)
        {
            if( found[i].first != found[i - 1].first )
            {
                for(int j = firstfound; j < i; j++)
                    linesbycount.push_back( make_pair( currentcount, found[j] ) );

                currentcount = 0;
                currentpath = found[i].first;
                firstfound = i;

                files_with_pattern ++;
            }
            currentcount ++;
        }

        for(int j = firstfound; j < (int)found.size(); j++)
                    linesbycount.push_back( make_pair( currentcount, found[j] ) );
    }

    //sorting from most patterns
    sort( linesbycount.begin(), linesbycount.end() );
    reverse( linesbycount.begin(), linesbycount.end() );

    //writing output to file
    std::ofstream resultstream;
    resultstream.open( resultfilepath );
    for(auto fline : linesbycount)
        resultstream << fline.second.first << ":" << fline.second.second.first << ":" << fline.second.second.second << "\n";
    resultstream.close();

    //writing logs to file
    std::ofstream logstream;
    logstream.open( logfilepath );
    for(auto const& log : threadlogs)
    {
        logstream << log.first << ": ";
        for(int i = 0; i < (int)log.second.size(); i++)
        {
            logstream << log.second[i];
            if( i + 1 != (int)log.second.size() )
                logstream << ",";
        }
        logstream << "\n";
    }
    logstream.close();

    //time counting
    timeval end_time, time_difference; gettimeofday( &end_time, 0 );
    timersub( &end_time, &start_time, &time_difference );

    //ending output
    std::cout << "Searched files: " << searched_files << "\n";
    std::cout << "Files with pattern: " << files_with_pattern << "\n";
    std::cout << "Patterns number: " << found.size() << "\n";
    std::cout << "Result file: " << resultfilepath << "\n";
    std::cout << "Log file: " << logfilepath << "\n";
    std::cout << "Used threads: " << threadnumber << "\n";
    std::cout << "Elapsed time: " << time_difference.tv_sec << "[s], " << time_difference.tv_usec / 1000 << "[ms]" << "\n";
}