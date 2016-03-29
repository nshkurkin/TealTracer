///
/// fileIO.h
/// --------
/// Nikolai Shkurkin
/// Utility Library
///

#ifndef ____util_fileIO__
#define ____util_fileIO__

#include <fstream>
#include <iostream>

#include <src/util/util_base.h>

namespace util {
    /// Reads the contents of `filename` into a string and returns it.
    std::string textFileRead(const std::string filename);
}

namespace util {
    /// Represents, you guessed it, a File Reader. It is a super basic way of
    /// quickly opening a file and reading line-by-line.
    struct FileReader {
        /// The file's source location.
        std::ifstream source;
        /// Whether or not a line can be read from the file.
        bool _hasNextLine;
        
        /// Creates a file reader using `fname`. It will output an error if
        /// the file could not be read.
        FileReader(const std::string fname) {
            source.open(fname.c_str(), std::ios_base::in);
            if (!source) {
                std::cerr << "Can't open " << fname << " for reading!\n";
                _hasNextLine = false;
            }
            _hasNextLine = true;
        }
        
        /// Indicates whether `readln` will return more content.
        bool hasNextLine() {return _hasNextLine;}
        
        /// Returns the next line of the file. Make sure to use this in
        /// conjunction with `hasNextLine`.
        std::string readln() {
            if (!source) {
                _hasNextLine = false;
                return std::string("");
            }
            else {
                std::string line;
                _hasNextLine = !std::getline(source, line).eof();
                return line;
            }
        }
    };
}

#endif // ____util_fileIO__
