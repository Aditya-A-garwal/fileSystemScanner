# File System Scanner

This file system scanner is a nifty command line tool to get folder/directory sizes in windows by [DumbleBots](https://dumblebots.com/). It can be alse be used to-
- find files and folder/ general navigation of your storage device.
- Find files and directories by their name 
- Navigate or find files and directories seperately

## Syntax

fss [-flag(s)] [[path] [filter_name]]\n"
					
## Flags:

-d show sub-directories

-f show individual files

-e show errors

-h for help

## Path

The relative or absoulute path to scan or search in

## Filter_name

Display only folders/directories and files which contain filter_name
					
## Examples:					

fss (show directories with sizes from current path)

fss -f (show directories and files with sizes from current path)

fss -d -f (show directories, sub-directories and files with sizes from current path)

fss -e (show directories with sizes from current path along with errors)
