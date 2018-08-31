#include <repack/rename/Driver.h>
#include <libutil/DefaultFilesystem.h>
#include <process/DefaultContext.h>

using libutil::DefaultFilesystem;

int
main(int argc, char **argv, char **envp)
{
	DefaultFilesystem filesystem = DefaultFilesystem();
	process::DefaultContext processContext = process::DefaultContext();

	repack::rename::Driver driver;
	return driver.run(&processContext, &filesystem);
}