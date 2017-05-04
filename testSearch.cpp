#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h>
#include <algorithm>
#include <vector>
#include <sstream>
#include <map>
#include <time.h>

using namespace std;

void ProcessDirectory(string directory,string word, ofstream& filePaths);
void ProcessFile(string file,string word, ofstream& filePaths);
void ProcessEntity(struct dirent* entity,string word, ofstream& filePaths);
bool hasEnding (string const &fullString, string const &ending);
int stringMatchCount(string file, string word);

string path = "/home/skon/books/";
int fileCount = 0;
int matchCount = 0;
int fileMatchCount = 0;
long long wordCount = 0;
string delimiters = " ,.;:?'\"()[]\r";
char carriage = '\r';
unsigned short pathNum = 0;
vector<string> stopList;
vector<string> wordList;
map<string, vector<string> > refs;//map of the text files
map<string, vector<string> >::iterator it;
int wordListCounter = 0;
int wordCounter = 0;
int stopCount = 0;
clock_t start = clock();


int main()
{
	ifstream fin;
	fin.open("stopwords.txt");
	string stopword;
	while(!fin.eof()) //make the list of stopwords
	{
		getline(fin, stopword);
		stopList.push_back(stopword);
		stopCount++;
	}
	fin.close(); //close the stopwords text file
	ofstream filePaths; //open the filepaths txt file
	filePaths.open("filePaths.txt", std::ios_base::app);
	string word;
	string directory = "";
	cout << "Word to search for: ";
	cin >> word;
	// Convert to lower case
	transform(word.begin(), word.end(), word.begin(), ::tolower);
	
	ProcessDirectory(directory,word,filePaths);
	
	cout << "Sort finished, outputting to text file." << endl;
	/*ofstream sortedWords;
	sortedWords.open("sortedWords.txt");
	for(it = refs.begin(); it != refs.end(); it++)
	{
		sortedWords << it->first;
		for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			sortedWords << *it2;
		}
		sortedWords << endl;
	}
	sortedWords.close();*/
	cout << "The word \"" << word << "\" found " << matchCount << " times in " << fileMatchCount << " books and " << wordCount << " words" << endl; 
	cout << "Total Books:" << fileCount << endl;
	filePaths.close();
	return 0;
}

bool hasEnding (string const &fullString, string const &ending) {
  if (fullString.length() >= ending.length()) {
    return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
  } else {
    return false;
  }
}

void ProcessDirectory(string directory, string word, ofstream& filePaths)
{
  string dirToOpen = path + directory;
  DIR *dir;
  dir = opendir(dirToOpen.c_str());

  //set the new path for the content of the directory
  path = dirToOpen + "/";

  //  cout << "Process directory: " << dirToOpen.c_str() << endl;

  if(NULL == dir)
    {
      cout << "could not open directory: " << dirToOpen.c_str() << endl;
      return;
    }
  struct dirent *entity;
  entity = readdir(dir);

  while(entity != NULL)
    {
      ProcessEntity(entity,word,filePaths);
      entity = readdir(dir);
    }

  //we finished with the directory so remove it from the path
  path.resize(path.length() - 1 - directory.length());
  closedir(dir);
}

void ProcessEntity(struct dirent* entity, string word, ofstream& filePaths)
{
  //find entity type
  if(entity->d_type == DT_DIR)
    {//it's an direcotry
      //don't process the  '..' and the '.' directories
      if(entity->d_name[0] == '.')
	{
	  return;
	}

      //it's an directory so process it
      ProcessDirectory(string(entity->d_name),word,filePaths);
      return;
    }

  if(entity->d_type == DT_REG)
    {//regular file
      ProcessFile(string(entity->d_name), word, filePaths);
      return;
    }

  //there are some other types
  //read here http://linux.die.net/man/3/readdir
  cout << "Not a file or directory: " << entity->d_name << endl;
}

void ProcessFile(string file, string word, ofstream& filePaths)
{
  ofstream log;
  log.open("log.txt", std::ios_base::app);
  string fileType = ".txt";
  if (hasEnding(file,fileType)) {
      fileCount++;
      if (word.length()>0) {
	int matches = stringMatchCount(file,word);
	if (1) {
	  fileMatchCount++;
	  matchCount += matches;
	  cout << path << file << endl; //when it encounters a new file
	  log << path << file << endl;
	  cout << wordListCounter << endl;
	  //cout << float((clock() - start)/500000)/60 << endl;
	  pathNum++;
	}
      }
  }
  log.close();
  //if you want to do something with the file add your code here
}

string getNext(string & line) {
  string next;
  size_t start = line.find_first_not_of(delimiters);
  if (start != string::npos) {
    //cout << ":" << start;
    size_t end = line.find_first_of(delimiters,start);
    if (end != string::npos) {
      //cout << "#" << end;
      // word with delimiters on both sides
      next = line.substr(start,end-start);
      line.erase(0,end+1);
    } 
	else {
      // word with delimiter only at start
      next = line.substr(start);
      line = "";
    }
  } else {
    // not delimiters found at all
    next = line;
    line = "";
  }
  //cout << "%" << next << "&" << line << endl;
  return next;
}

int stringMatchCount(string file, string word) {
  ifstream infile;
  int count = 0;
  string line,w;

  try{
    string fileWithPath = path+file;
    infile.open(fileWithPath.c_str());
    //    cout << "open:" << fileWithPath << endl;
	int lines = 0;
	int words;
    while(!infile.eof() && lines++ < 10000)
	{
		words = 0;
		// normalize to lower case
		getline(infile,line);
		//cout << line << endl;
		while (line.length()>0 && words++ < 100) 
		{
			bool wordPresent = false;
			bool stopPresent = false;
			//string position = "@";
			//position += to_string(pathNum);
			//position += "#";
			//position += to_string(lines);
			unsigned short book = pathNum;
			unsigned short position = lines;
			w = getNext(line);
			transform(w.begin(), w.end(), w.begin(), ::tolower);
			//cout << "*" << w << "*";
			wordCount++;
			for(string::iterator it = w.begin(); it < w.end(); it++)
			{
				if(int(*it) > 122 || int(*it) < 65)
				{
					stopPresent = true;
				}
			}
			for(int i = 0; i < stopCount; i++)
			{
				if(stopList[i] == w)
				{
					stopPresent = true;
				}
			}
			if(stopPresent == false)
			{
				//refs[w].push_back(position);
				//wordListCounter++;
				ofstream fout;
				string wordfile = "/home/students/seavera/project6/Search/wordfiles/";
				wordfile += w;
				wordfile += ".bin";
				fout.open(wordfile, ios::out | ios::binary | ios::app);
				fout.write((char*)&book, sizeof(unsigned short));
				fout.write((char*)&position, sizeof(unsigned short));
				fout.close();
				wordListCounter++;
			}
			
		}
    }

    infile.close();
  }catch(ifstream::failure e){
    //cout<<e<<endl;
  }
  return count;
}