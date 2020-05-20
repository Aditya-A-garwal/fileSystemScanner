#include <string>
#include <iostream>
#include <filesystem>

#define NUMDIGITS	13

using namespace std;
using namespace filesystem;

void printIndent(int s) {
	for(int i = 0; i <= min(s, 6); i++) cout << "    ";	
	if(s > 6) {		
		for(int i = 6; i <= s; i++) cout << "..";
	}		
}

long getSizeOf(directory_entry entry) {	
	
	if(entry.is_directory() == 1) {
		directory_iterator iter(entry.path());
				
		long size = 0;

		while(iter != end(iter)) {
			directory_entry ent = *iter;			
			
			size += ent.file_size();
			if(ent.is_directory() == 1) size += getSizeOf(ent);
			
			iter++;		
		}
		
		return size;
	}
	
	else return entry.file_size();	
	
}

void printContentA(directory_iterator iter, int s, bool recur) {	
	directory_entry ent;
	unsigned long size = 0;
	unsigned int numFiles = 0;

	while(iter != end(iter)) {
		ent = *iter;											
		
	    if(ent.is_directory() == 1) {				
			cout << setfill(' ') << setw(NUMDIGITS) << getSizeOf(ent);
			printIndent(s);	
			cout << "<" << ent.path().stem().string() << ">" << endl;		
			if(recur) printContentA(directory_iterator(ent.path()), (s+1), 1);
		}		
		
		else if(ent.is_directory() != 1) {
			numFiles++;
			size += ent.file_size();					
		}
		iter++;		
	}
	if(numFiles != 0) {
		cout << setfill(' ') << setw(NUMDIGITS) << size;
		printIndent(s);
		cout << "<" << numFiles << " files>\t" << endl;				
	}	
}

void printContentB(directory_iterator iter, int s, bool recur) {	
	
	directory_entry ent;

	while(iter != end(iter)) {
		ent = *iter;	
		iter++;
		cout << setfill(' ') << setw(NUMDIGITS) << getSizeOf(ent);			
				
		printIndent(s);									
		
		if(ent.is_directory() != 1) {		
			cout << ent.path().stem().string() << ent.path().extension().string() << endl;				
		}
		
		else if(ent.is_directory() == 1) {		
			cout << "<" << ent.path().stem().string() << ">" << endl;					
			if(recur) printContentB(directory_iterator(ent.path()), (s+1), 1);			
		}
	}			
}

int main(int argc, char* argv[]) {					
	
	path p = ".";
	int mode = 0;		
	bool isRecur = 0;
	
	if(argc >= 2) {
		for(int i = 1; i < argc; i++) {					
			if((argv[i])[0] == '-') {				
				if((argv[i])[1] == 'd') isRecur = 1;
				else if((argv[i])[1] == 'f') mode = 1;
				else if((argv[i])[1] == 'h') {
					cout << "\nfss [-h] [-f] [-d] [path]\n";
					cout << "fss [-h] [-d] [-f] [path]\n";
					cout << "fss [-h] [-f] [path] [-d]\n";
					cout << "fss [-h] [-d] [path] [-f]\n\n";					
					cout << "-d recursively prints directories\n";
					cout << "-f expands files\n";
					cout << "-h for help\n" << endl;
					return 0;
				}
			}				
			else {
				p = argv[i];			
			}
		}			
	}					
	
	cout << "\nFilesystem Scanner v0.9" << endl;					
	cout << "from dumblebots.com" << endl;							
	
	cout << absolute(p).string() << "\\" << endl;					
		
	directory_iterator myDir(p);		    
	
	if(mode == 0) printContentA(myDir, 0, isRecur);
	else if(mode == 1) printContentB(myDir, 0, isRecur);
	directory_entry ent;

}
