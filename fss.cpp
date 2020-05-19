#include <string>
#include <iostream>
#include <filesystem>

#define DEFAULT	0
#define RECURSE	1	
#define DIRFILE	2

using namespace std;
using namespace filesystem;

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


void printContent0(directory_iterator iter, int s) {	
	directory_entry ent;
	int size = 0;
	int numFiles = 0;

	while(iter != end(iter)) {
		ent = *iter;											

	    if(ent.is_directory() == 1) {				
			cout << setfill(' ') << setw(10) << getSizeOf(ent);
			if(s <= 6) { 
				for(int i = 0; i <= s; i++) cout << "    ";
			}
			else {
				for(int i = 0; i <= 6; i++) cout << "    ";
				for(int i = 6; i <= s; i++) cout << "..";
			}		
			cout << "<" << ent.path().stem().string() << ">" << endl;													
		}		
		
		else if(ent.is_directory() != 1) {
			numFiles++;
			size += ent.file_size();										
		}			
		
		iter++;			
	}		

	if(numFiles != 0) {
		cout << setfill(' ') << setw(10) << size;
		if(s <= 6) { 
			for(int i = 0; i <= s; i++) cout << "    ";
		}
		else {
			for(int i = 0; i <= 6; i++) cout << "    ";
			for(int i = 6; i <= s; i++) cout << "..";
		}		
		cout << "<" << numFiles << " files>\t" << endl;				
	}			
}


void printContent1(directory_iterator iter, int s) {		
	directory_entry ent;
	int size = 0;
	int numFiles = 0;

	while(iter != end(iter)) {
		ent = *iter;											

	    if(ent.is_directory() == 1) {				
			cout << setfill(' ') << setw(10) << getSizeOf(ent);
			if(s <= 6) { 
				for(int i = 0; i <= s; i++) cout << "    ";
			}
			else {
				for(int i = 0; i <= 6; i++) cout << "    ";
				for(int i = 6; i <= s; i++) cout << "..";
			}		
			cout << "<" << ent.path().stem().string() << ">" << endl;											
			printContent1(directory_iterator(ent.path()), (s+1));			
		}		
		
		else if(ent.is_directory() != 1) {
			numFiles++;
			size += ent.file_size();										
		}			
		
		iter++;			
	}		

	if(numFiles != 0) {
		cout << setfill(' ') << setw(10) << size;
		if(s <= 6) { 
			for(int i = 0; i <= s; i++) cout << "    ";
		}
		else {
			for(int i = 0; i <= 6; i++) cout << "    ";
			for(int i = 6; i <= s; i++) cout << "..";
		}		
		cout << "<" << numFiles << " files>\t" << endl;				
	}
}


void printContent2(directory_iterator iter, int s) {	
	
	directory_entry ent;

	while(iter != end(iter)) {
		ent = *iter;						
		cout << setfill(' ') << setw(10) << getSizeOf(ent);			
				
		if(s <= 6) { 
			for(int i = 0; i <= s; i++) cout << "    ";
		}
		else {
			for(int i = 0; i <= 6; i++) cout << "    ";
			for(int i = 6; i <= s; i++) cout << "..";
		}									
		
		if(ent.is_directory() != 1) {		
			cout << ent.path().stem().string() << ent.path().extension().string() << endl;				
		}
		
		else if(ent.is_directory() == 1) {		
			cout << "<" << ent.path().stem().string() << ">" << endl;					
			printContent2(directory_iterator(ent.path()), (s+1));			
		}
		
		iter++;		
	}			
}

int main(int argc, char* argv[]) {					
	
	cout << "Filesystem Scanner v0.9" << endl;					
	cout << "from dumblebots.com" << endl;					
	
	path p;
	
	if(argc == 1 || (argv[1])[0] == '-') p = ".";
	else p = argv[1];
	
	cout << p.string() << "\\" << endl;					
		
	directory_iterator myDir(p, std::filesystem::directory_options::skip_permission_denied);		    
	printContent0(myDir, 0);
	directory_entry ent;

}
