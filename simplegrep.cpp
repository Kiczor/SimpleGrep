#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <string>
#include <regex>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sys/time.h>

std::string logfilepath, resultfilepath, dirpath, pattern;
int threads;

//global stats
int searched_files, files_with_pattern;
timeval start_time;

typedef std::pair<std::string, std::pair<int, std::string> > fileline;

std::vector<fileline> found;

void SeekInFile( std::string path, std::string pattern )
{
    std::regex patternregex( pattern );

    std::ifstream filestream(path);
    std::string line;

    int count = 0;
    while( getline( filestream, line ) )
    {
        count ++;
        if( std::regex_search( line, patternregex ) )
        {
            //add to results (mutex chyba tu powinien byc)
            found.push_back( make_pair( path, make_pair( count, line ) ) );
        }
    }

    return;
}

int main(int argc, char** argv)
{
    gettimeofday(&start_time, 0);

    dirpath = "";
    logfilepath = "logfile.txt";
    resultfilepath = "resultfile.txt";
    threads = 4;

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
            case 'l':
                logfilepath = optarg;
            case 'r':
                resultfilepath = optarg;
            case 't':
                threads = atoi( optarg );
            case '?':
                break;
        }
    }

    pattern = argv[ argc - 1 ];

    //iterating directory
    for (const auto& direntry : std::filesystem::recursive_directory_iterator("."))
    {
        std::cout << direntry << std::endl;
        SeekInFile( direntry.path(), pattern );
        searched_files += 1;
    }

    sort( found.begin(), found.end() );

    //debugowanie
    for(int i = 0; i < (int)found.size(); i++)
        std::cout << found[i].first << " " << found[i].second.first << " " << found[i].second.second << "\n";

    //counting different files with pattern
    std::vector< std::pair< int, fileline > > linesbycount;

    int currentcount = 0;
    std::string currentpath = "";
    if( found.size() > 0 )
    {
        currentpath = found[1].first;
        currentcount = 1;
        for(int i = 1; i < (int)found.size(); i++)
        {
            if( found[i].first != found[i - 1].first )
            {
                linesbycount.push_back( make_pair( currentcount, found[i - 1] ) );

                currentcount = 0;
                currentpath = found[i].first;

                files_with_pattern ++;
            }
            currentcount ++;
        }

        linesbycount.push_back( make_pair( currentcount, found[ (int)found.size() - 1 ] ) );
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

    //time counting
    timeval end_time, time_difference; gettimeofday( &end_time, 0 );
    timersub( &end_time, &start_time, &time_difference );

    //ending output
    std::cout << "Searched files: " << searched_files << "\n";
    std::cout << "Files with pattern: " << files_with_pattern << "\n";
    std::cout << "Patterns number: " << found.size() << "\n";
    std::cout << "Result file: " << resultfilepath << "\n";
    std::cout << "Log file: " << logfilepath << "\n";
    std::cout << "Used threads: " << threads << "\n";
    std::cout << "Elapsed time: " << time_difference.tv_sec << "[s], " << time_difference.tv_usec / 1000 << "[ms]" << "\n";
}