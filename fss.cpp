#include <string>
#include <iostream>
#include <filesystem>

using namespace std;
using namespace filesystem;

long getSizeOf(directory_iterator iter) {
	long size = 0;
	
	directory_entry ent;

	while(iter != end(iter)) {
		ent = *iter;			
		
		if(ent.is_directory() == 1) { 
			size += getSizeOf(directory_iterator(ent.path()));
		}
		else {
			size += ent.file_size();
		}		
		
		iter++;		
	}
	
	return size;
	
}

void printContent(directory_iterator iter, int s) {	
	
	directory_entry ent;

	while(iter != end(iter)) {
		ent = *iter;
		int size = ent.file_size();
		
		if(ent.is_directory() == 1) { 
			size = getSizeOf(directory_iterator(ent.path()));
		}
		cout << size;
		
		for(int i = 0; i <= s; i++) cout << "\t";
		
		cout << ent.path() << endl;	
		
		if(ent.is_directory() == 1) { 
			//cout << size << "\t\t\t";		
			//cout << ent.path() << endl;	
			printContent(directory_iterator(ent.path()), (s+1));
		}
		
		iter++;		
	}			
}

int main(int argc, char* argv[]) {				
	
	if(argc > 2) return 0;
	
	cout << "printing contents of " << argv[1] << "\n" << endl;	
		
	directory_iterator myDir(argv[1]);		    
	printContent(myDir, 0);
	directory_entry ent;

}
