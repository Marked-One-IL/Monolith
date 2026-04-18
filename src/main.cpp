#include <Lexer/Generator.hpp>
#include <Parser/Generator.hpp>
#include <Semantic/IdentifierLinker.hpp>
#include <Semantic/TypeChecking.hpp>
#include <x86Gen/Generator.hpp>
#include <Helper/Helper.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <format>
#include <cmath>
#include <filesystem>

// In debug mode.
// The code will not compile.
// But only finish until the stage of Codegen (including Codegen) and exit.
// Only the asm file will be produced.

int main(int argc, char** argv)
{
#ifdef NDEBUG
    if (argc < 3)
    {
        std::cout << "mon [Input filename (No extension)] [Output filename (No extension)] ... [optional .lib files (Have extension) or also optional -keepTemps in the middle]";
        return 1;
    }
#endif

#ifdef NDEBUG
    std::string input = std::format("{}.mon", argv[1]);
    std::string outputAsm = std::format("{}.asm", argv[2]);
    std::string outputObj = std::format("{}.obj", argv[2]);
#else
    std::filesystem::current_path(std::filesystem::path(CMAKE_PROJECT_ROOT) / "Debug_Mon");
    std::string input = "main.mon";
    std::string outputAsm = "out.asm";
#endif

    try
    {
#ifdef NDEBUG
        bool keepTemps = false;

        std::string libs;
        for (int i = 3; i < argc; i++)
        {
            if (argv[i] == std::string_view("-keepTemps")) keepTemps = true;
            else
            {
                std::filesystem::path fullPath = std::filesystem::weakly_canonical(argv[i]);
                libs += std::format("\"{}\" ", fullPath.string());
            }
        }
#endif
        std::fstream output(outputAsm, std::ios::out | std::ios::trunc);
        std::string content = Helper::extractFileContent(input);

        Lexer::Generator lexer(std::move(content), input);
        Parser::Generator parser(lexer);
        Semantic::IdentifierLinker::link(parser);
        Semantic::TypeChecking::check(parser);
        x86Gen::Generator codeGen(parser);
        
        output << codeGen.getAsm();
        output.close(); // Must close before assembling.

#ifdef NDEBUG
        std::string cmdMasm = std::format(R"(C:\mon\bin\ml.exe /c /coff {})", outputAsm);
        std::string cmdLink = std::format(
R"(C:\mon\bin\link.exe /subsystem:console /entry:mainCRTStartup /OPT:NOREF /OPT:NOICF /STACK:1048576 {} {} /libpath:"C:\mon\lib" user32.lib gdi32.lib libcmt.lib libvcruntime.lib libucrt.lib legacy_stdio_definitions.lib kernel32.lib)",
        outputObj, libs);

        std::system(cmdMasm.c_str());
        std::system(cmdLink.c_str());
        if (not keepTemps)
        {
            std::filesystem::remove(std::filesystem::path(outputAsm));
            std::filesystem::remove(std::filesystem::path(outputObj));
        }
#endif
    }
    catch (const std::filesystem::filesystem_error& error)
    {
#ifdef NDEBUG
        std::filesystem::remove(std::filesystem::path(outputAsm));
#endif
        std::cout << error.what() << '\n';
        return 1;
    }
    catch (const std::system_error& error)
    {
#ifdef NDEBUG
        std::filesystem::remove(std::filesystem::path(outputAsm));
#endif
        std::cout << error.what() << '\n';
    }
    catch (const std::exception& error)
    {
#ifdef NDEBUG
        std::filesystem::remove(std::filesystem::path(outputAsm));
#endif
        std::cout << error.what() << '\n';
        return 1;
    }

    return 0;
}
