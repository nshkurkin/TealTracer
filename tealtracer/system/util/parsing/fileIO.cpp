///
/// fileIO.cpp
/// ----------
/// Nikolai Shkurkin
/// Utility Library
///

#include "fileIO.h"

std::string util::textFileRead(const std::string fileName) {
    FILE *fp;
    char *content = NULL;
    const char *fn = fileName.c_str();
    int count = 0;
    if (fn != NULL) {
        fp = fopen(fn, "rt");
        if (fp != NULL) {
            fseek(fp, 0, SEEK_END);
            count = (int)ftell(fp);
            rewind(fp);
            if (count > 0) {
                content = (char *)malloc(sizeof(char) * (count+1));
                count = (int)fread(content,sizeof(char),count,fp);
                content[count] = '\0';
            }
            fclose(fp);
        } else {
            std::cout << "error loading " << fn << "\n";
        }
    }
    return std::string(content);
}
