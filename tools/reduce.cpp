
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>

#include "cppcheck.h"


class CppcheckExecutor : public ErrorLogger {
private:
    CppCheck cppcheck;
    const std::string line;
    bool foundLine;

public:
    CppcheckExecutor(const std::string &l)
        : ErrorLogger()
        , cppcheck(*this,false)
        , line(':' + l + ']')
        , foundLine(false) {
    }

    bool run(const char filename[]) {
        foundLine = false;
        Settings settings;
        settings.addEnabled("all");
        settings.inconclusive = true;
        settings._force = true;
        cppcheck.settings() = settings;
        cppcheck.check(filename);
        return foundLine;
    }

    void reportOut(const std::string &outmsg) { }
    void reportErr(const ErrorLogger::ErrorMessage &msg) {
        if (msg.toString(false).find(line) != std::string::npos) {
            foundLine = true;
            cppcheck.terminate();
        }
    }
    void reportProgress(const std::string &filename, const char stage[], const unsigned int value) { }
};


static bool test(CppcheckExecutor &cppcheck, const char *filename, const std::vector<std::string> &filedata, const std::size_t line1, const std::size_t line2)
{
    std::string path(filename);
    if (path.find_first_of("\\/") != std::string::npos)
        path = path.erase(1 + path.find_last_of("\\/"));
    else
        path.clear();

    const std::string tempfilename(path + "__temp__" + std::strrchr(filename,'.'));
    std::ofstream fout(tempfilename.c_str());
    for (std::size_t i = 0; i < filedata.size(); i++)
        fout << ((i>=line1 && i<=line2) ? "" : filedata[i]) << std::endl;
    fout.close();
    return cppcheck.run(tempfilename.c_str());
}

static bool test(CppcheckExecutor &cppcheck, const char *filename, const std::vector<std::string> &filedata, const std::size_t line)
{
    return test(cppcheck, filename, filedata, line, line);
}

static void printstr(const std::vector<std::string> &filedata, int i1, int i2)
{
    std::cout << filedata.size();
    for (int i = i1; i < i2; ++i)
        std::cout << i << ":" << filedata[i] << std::endl;
}

int main(int argc, char *argv[])
{
    std::cout << "cppcheck tool that reduce code for a false positive" << std::endl;

    if (argc != 3) {
        std::cerr << "Syntax: " << argv[0] << " filename line" << std::endl;
        return EXIT_FAILURE;
    }

    const char * const filename = argv[1];
    const char * const linenr   = argv[2];

    std::cout << "make sure false positive can be reproduced" << std::endl;

    // Execute Cppcheck on the file..
    CppcheckExecutor cppcheck(linenr);
    if (!cppcheck.run(filename)) {
        std::cerr << "Can't reproduce false positive at line " << linenr << std::endl;
        return EXIT_FAILURE;
    }

    // Read file..
    std::vector<std::string> filedata;
    {
        std::ifstream fin(filename);
        std::string line;
        bool blockComment = false;
        while (std::getline(fin,line)) {
            // replace various space characters with space
            for (std::string::size_type pos = 0; pos < line.size(); ++pos) {
                if (line[pos] & 0x80 || std::isspace(line[pos]))
                    line[pos] = ' ';
            }

            // remove block comments TODO: Handle /* inside strings
            if (blockComment) {
                if (line.find("*/") == std::string::npos)
                    line.clear();
                else {
                    if (line.find("*/") + 2U == line.size())
                        line.clear();
                    else
                        line = line.substr(line.find("*/") + 2U);
                    blockComment = false;
                }
            }
            while (!blockComment && line.find("/*") != std::string::npos) {
                std::string::size_type pos = line.find("/*");
                if (line.find("*/",pos) == std::string::npos) {
                    blockComment = true;
                    if (pos==0)
                        line.clear();
                    else
                        line = line.substr(0,pos);
                } else {
                    blockComment = false;
                    line = line.erase(pos, 2U + line.find("*/", pos) - pos);
                }
            }

            // Remove // comments
            if (line.find("//") != std::string::npos)
                line = line.substr(0, line.find("//"));

            // empty line
            if (line.find_first_not_of(" ") == std::string::npos)
                line.clear();
            else {
                const std::string::size_type pos = line.find_first_not_of(" ");

                // remove spaces before leading #
                if (line[pos]=='#')
                    line = line.substr(pos);
            }

            // remove trailing spaces
            while (!line.empty() && std::isspace(line[line.size()-1U]))
                line = line.substr(0, line.size() - 1U);

            filedata.push_back(line);
        }
    }


    // Write resulting code..
    if (!test(cppcheck, filename, filedata, ~0)) {
        std::cerr << "Cleanup failed." << std::endl;
        return EXIT_FAILURE;
    }

    // Remove includes..
    for (std::size_t i = 0; i < filedata.size(); i++) {
        if (filedata[i].compare(0,8,"#include")==0) {
            if (test(cppcheck, filename, filedata, i)) {
                filedata[i].clear();
                std::cout << "removed #include at line " << i << std::endl;
            } else {
                std::cout << "kept #include at line " << i << std::endl;
            }
        }
    }

    // Remove blocks of code..
    for (std::size_t i = 0; i < filedata.size(); i++) {
        const std::string line = (i==0U&&filedata.empty()) ? std::string(";") : filedata[i];
        if (line.empty())
            continue;

        const char startChar = line[0];
        const char endChar   = line[line.size() - 1U];

        // some kind of single line declaration
        if (std::isalpha(startChar) && endChar==';') {
            if (test(cppcheck, filename, filedata, i)) {
                filedata[i].clear();
                std::cout << "removed declaration at line " << i << std::endl;
            } else {
                std::cout << "kept declaration at line " << i << std::endl;
            }
        }

        // remove a function body below a '}'
        bool decl = bool(!std::isspace(startChar) && (endChar=='}' || endChar==';'));
        while (decl) {
            decl = false; // might be set to true below
            std::size_t pos = ++i;
            while (pos < filedata.size() && filedata[pos].empty())
                ++pos;
            if ((pos+2U < filedata.size()) && (std::isalpha(filedata[pos].at(0)))) {

                // does code block start with "{"
                std::size_t pos2 = pos;
                if (filedata[pos].find("(") != std::string::npos && filedata[pos].find(")")==std::string::npos) {
                    ++pos2;
                    while (pos2+2U < filedata.size() && !filedata[pos2].empty() && filedata[pos2].find_first_of("(){}") == std::string::npos)
                        ++pos2;
                    if (filedata[pos2].find_first_of("({}")!=std::string::npos || filedata[pos2].find(")") == std::string::npos)
                        break;
                }
                if (pos2+2U >= filedata.size() || filedata[pos2+1U] != "{")
                    break;
                pos2 += 2;

                // find end of block..
                while ((pos2 < filedata.size()) && (filedata[pos2].empty() || std::isspace(filedata[pos2].at(0))))
                    ++pos2;

                // does block of code end with a '}'
                if ((pos2 < filedata.size()) && (filedata[pos2] == "}" || filedata[pos2] == "};")) {
                    if (test(cppcheck, filename, filedata, pos, pos2)) {
                        for (i = pos; i <= pos2; i++)
                            filedata[i].clear();
                        std::cout << "removed block of code at lines " << pos << "-" << pos2 << std::endl;
                        decl = true;
                    } else {
                        std::cout << "kept block of code at lines " << pos << "-" << pos2 << std::endl;
                    }
                }
            }
        }
    }

    // Write resulting code..
    {
        const std::string outfilename(std::string("__out__") + std::strrchr(filename,'.'));
        std::ofstream fout(outfilename.c_str());
        for (std::size_t i = 0; i < filedata.size(); i++) {
            if (!filedata[i].empty())
                fout << filedata[i] << std::endl;
        }
    }

    return EXIT_SUCCESS;
}
