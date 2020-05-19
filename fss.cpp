#include <string>
#include <iostream>
#include <filesystem>

using namespace std;
using namespace filesystem;

void prntLatest() {

}

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
		int size = 0;
		
		if(s <= 6) { 
			for(int i = 0; i <= s; i++) cout << "    ";
		}
		else {
			for(int i = 0; i <= 6; i++) cout << "    ";
			for(int i = 6; i <= s; i++) cout << "..";
		}
				
		cout << ent.path().stem().string();		
		
		if(ent.is_directory() != 1) {
			size = ent.file_size();							
			cout << ent.path().extension().string() << "\t\t\t" << size << endl;			
		}
		
		else if(ent.is_directory() == 1) {		
			size = getSizeOf(directory_iterator(ent.path()));
			cout << "\\\t\t\t" << size << endl;			
			printContent(directory_iterator(ent.path()), (s+1));			
		}
		
		iter++;		
	}			
}

int main(int argc, char* argv[]) {					
	
	cout << "\nFilesystem Scanner" << endl;					
	cout << "www.dumblebots.com" << endl;					
	cout << "Begin scanning\n\n" << endl;					
	
	path p;
	
	if(argc == 1 || (argv[1])[0] == '-') p = ".";
	else p = argv[1];
	
	cout << p.string() << "\\" << endl;					
		
	directory_iterator myDir(p, std::filesystem::directory_options::skip_permission_denied);		    
	printContent(myDir, 0);
	directory_entry ent;

}
