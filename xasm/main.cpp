#include "xasm.hpp"

void PrintUsage ()
{
    printf ( "Usage:\tXASM Source.XASM [Executable.XSE]\n" );
    printf ( "\n" );
    printf ( "\t- File extensions are not required.\n" );
    printf ( "\t- Executable name is optional; source name is used by default.\n" );
}

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		PrintUsage();
		return 0;
	}

	string file_name = argv[1];

	xscript::xasm::xasm xm;
	xm.complier(file_name);
}
