#include <iostream>
#include <array>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include<algorithm>
using namespace std;

#define NUM_OF_CHAR 98
#define MAX_TABLE_SIZE 5000
#define CMAP_SIZE 392
#define CHAR_OFFSET -29
#define STRING_BUFFER_SIZE 5000
#define GAP 1000

array<int, NUM_OF_CHAR> cMap = {0};
vector<array<int, NUM_OF_CHAR>> occuranceTable;
unordered_set<int> queryID;
ifstream INPUT_FILE;
ifstream INDEX_FILE;
bool constructIndex = false;
int recordNum = 0;


int charToIndex(char& c) {
    if ((int) c >= 32) { 
        return c + CHAR_OFFSET;
    } else if ((int) c == 9) {
        return 0;
    } else if ((int) c  == 10) {
        return 1;
    } else {
        return 2;
    }
}


void buildCMap(ifstream& inputFile, string indexFileName, int mode) {
    ofstream indexFile;
    if (mode == 1) indexFile.open(indexFileName);
    int curPos = 0;
    array<char, STRING_BUFFER_SIZE> buffer;
    while (true) {
        inputFile.read(buffer.data(), STRING_BUFFER_SIZE);
        streamsize bytesRead = inputFile.gcount();
        if (bytesRead == 0) {
            break; 
        }
        for (streamsize i = 0; i < bytesRead; i++) {
            char byte = buffer[i];
            if (curPos == GAP) {
                curPos = 1;
                if (constructIndex) {
                    indexFile.write(reinterpret_cast<const char*>(cMap.data()), cMap.size() * sizeof(int));
                } else {
                    array<int, NUM_OF_CHAR> temp = cMap;
                    occuranceTable.push_back(temp);
                }
            } else {
                curPos++;
            }
            int index = charToIndex(byte);
            cMap[index]++;
        }
    }
    for (int i = 1; i < cMap.size(); i++) {
        cMap[i] += cMap[i - 1];
    }
    indexFile.write(reinterpret_cast<const char*>(cMap.data()), cMap.size() * sizeof(int));
    inputFile.clear();
    inputFile.seekg(0, ios::beg);
    if (mode == 1) indexFile.close();
}


streamoff checkSize(ifstream& inputFile) {
    inputFile.seekg (0, ios::end);
    streampos end = inputFile.tellg();
    streamoff size = end;
    inputFile.seekg (0, ios::beg);
    return size;
}


bool fileExists(const string& filename) {
    ifstream file(filename);
    return file.good();
}


void readArrayFromFile(array<int, NUM_OF_CHAR>& arr, ifstream& inputFile, streamoff pos) {
    if (inputFile.eof()) inputFile.clear(); 
    inputFile.seekg(pos, ios::beg);


    array<char, CMAP_SIZE> buffer;
    inputFile.read(buffer.data(), CMAP_SIZE);

    if (inputFile.gcount() < CMAP_SIZE) {
        cerr << "incomplete data read at position " << pos << endl;
        cerr << "Bytes read: " << inputFile.gcount() << endl;
        exit(1);
    }

    for (int i = 0; i < NUM_OF_CHAR; ++i) {
        arr[i] = *reinterpret_cast<int*>(buffer.data() + i * sizeof(int));
    }
}


int increOcc(int size, int count, streamoff offset, char cur) {
    if (size == 0) return count;

    if (INPUT_FILE.eof()) INPUT_FILE.clear();
    vector<char> buffer(size);
    INPUT_FILE.seekg(offset, ios::beg);
    INPUT_FILE.read(buffer.data(), size);
    for (char& c: buffer) {
        if (c == cur) count ++;
    }
    return count;
}


int occuranceFunction(char c, int pos, int mode) {
    int index = pos/GAP;
    int count = 0;
    int cMapIndex = charToIndex(c);
    if (!constructIndex) {
        if (index == 0) {
            count = 0;
        } else {
            count = occuranceTable[index - 1][cMapIndex];
        }
    } else {
        array<int, NUM_OF_CHAR> buffer;
        if (index == 0) {
            count = 0;
        } else {
            readArrayFromFile(buffer, INDEX_FILE, (index - 1)*CMAP_SIZE);
            count = buffer[cMapIndex];
        }

    }
    if (mode == 1) {
        count = increOcc(pos - index*GAP, count, index*GAP, c);
    }
    if (cMapIndex != 0){
        count += cMap[cMapIndex - 1];
    }
    return count;
}

array<int, 3> backwardSearch(string& s) {
    char cur = s[s.length() - 1];
    int index = charToIndex(cur);
    int first = index == 0? 0 : cMap[index - 1];
    int last = cMap[index];
    int i = s.length() - 2;
    while (first < last && i >= 0) {
        cur = s[i];
        first = occuranceFunction(cur, first, 1);
        last = occuranceFunction(cur, last, 1);
        i --;
    }
    if (last <= first) return array<int, 3> {0, 0 ,0};
    return array<int, 3> {1, first, last};
}


array<int, 2> findHead(int pos) {

    if (INPUT_FILE.eof()) INPUT_FILE.clear();
    int prevOccIndex = pos/GAP;
    int size = pos - prevOccIndex*GAP + 1;
    vector<char> buffer(size);
    INPUT_FILE.seekg(prevOccIndex*GAP, ios::beg);
    INPUT_FILE.read(buffer.data(), size);
    char cur = buffer[buffer.size() - 1];
    int index = occuranceFunction(cur, pos, 0);
    for (int i = 0; i < buffer.size() - 1; i ++) {
        if (buffer[i] == cur) index ++;
    }
    return array<int, 2> {cur ,index};
}

int bsOut_forNum(int pos) {
    bool begin = false;
    bool previous = false;
    string output ="";
    string debug = "";
    while (true) {
        array<int, 2> curChar = findHead(pos);
        if (curChar[0] == 91) {
            if (begin) break;
            else previous = true;
        }

        if (curChar[0] == 93) {
            begin = true;
        } else if (begin) {
            output += (char) curChar[0];
        }      
        pos = curChar[1];
        debug += (char) curChar[0];
    }
    
    reverse(output.begin(), output.end());
    int numid = -1;
    if (previous) {
        numid = stoi(output) + 1;
  
    } else {
   
        numid = stoi(output);
    }
    return numid;
}


string bsOut(int pos, int mode) {
    bool begin = false;
    string output ="";
    while (true) {
        array<int, 2> curChar = findHead(pos);
        if (curChar[0] == 91) {
            break;
        }
        if (mode == 1) {
            if (curChar[0] == 93) {
                begin = true;
            } else if (begin) {
                output += (char) curChar[0];
            }      
        } else {
            output += (char) curChar[0];
        }
        pos = curChar[1];
    }
    if (mode == 0) output += '[';
    reverse(output.begin(), output.end()); 
    return output;
}

bool isNumber(const string& str) {
    for (char const &c : str) {
        if (isdigit(c) == 0) return false;
    }
    return true;
}

unordered_set<int> backwardSearchString(string& s, unordered_set<int>& set, int mode) {
    array<int, 3> checkBs = backwardSearch(s);
    unordered_set<int> output;
    if (checkBs[0] == 0) return output;

    for (int i = checkBs[2] - 1; i >= checkBs[1]; i --) {
        int numid = -1;
        if (isNumber(s)) {
            numid = bsOut_forNum(i);
        } else {
            numid = stoi(bsOut(i, 1));
            if (s[0] == '[') numid ++;
        }

        if (mode == 1 && set.find(numid) == set.end()) {
            continue;
        }
        output.insert(numid);
    }
    return output;
}


void backwardSearchRecord(int id, string &s) {
    string stringID = "[" + to_string(id + 1) + "]";
    array<int, 3> serachResult = backwardSearch(stringID); 
    if (serachResult[0] == 0) {
        stringID = "[" + to_string(id + 1 - recordNum) + "]";
        serachResult = backwardSearch(stringID); 
    }
    s = bsOut(serachResult[1], 0);
}


void findQuery(vector<string> query) {

    queryID = backwardSearchString(query[0], queryID, 0);
    for (int i = 1; i < query.size(); i ++) {
        queryID = backwardSearchString(query[i], queryID, 1);
        if (queryID.empty()) break;
    }
}


void outPutQuery(){
    vector<int> outVec;
    for (const int& num: queryID) {
        
        outVec.push_back(num);
    }
    sort(outVec.begin(), outVec.end());
    for (int i = 0; i < outVec.size(); i++) {
        string out = "";
        backwardSearchRecord(outVec[i], out);
        cout<<out<<endl;
    }

}

int main(int argc, char* argv[]) {
    if (argc < 4 || argc > 8) {
        cerr << "Usage: " << argv[0] << " <bwt_file> <index_file> <query1> [<query2> ... <query5>]" << endl;
        return 1;
    }

    string bwtFilename = argv[1];
    string indexFilename = argv[2];
    vector<string> stringQuery;

    for (int i = 3; i < argc; i++) {
        string s = argv[i];
        if (s.front() == '"' && s.back() == '"') {
            s = s.substr(1, s.size() - 2);
        }
        stringQuery.push_back(s);
    }

    INPUT_FILE.open(bwtFilename, ios::binary);
    streamoff size = checkSize(INPUT_FILE);
    streamoff cMapPos = 0;
    int tableNum = (size - 1)/GAP;

    if (tableNum > MAX_TABLE_SIZE) {
        constructIndex = true;
        cMapPos = tableNum*CMAP_SIZE;

    }

    if (!INPUT_FILE.is_open()) {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    if (!constructIndex) {
        buildCMap(INPUT_FILE, indexFilename, 0);
    } else if (fileExists(indexFilename)) {

        ifstream readFromIndexFile(indexFilename, ios::binary);
        readArrayFromFile(cMap, readFromIndexFile, cMapPos);
        readFromIndexFile.close();
    } else {

        buildCMap(INPUT_FILE, indexFilename, 1);
    }
    recordNum = cMap[91 + CHAR_OFFSET] - cMap[90 + CHAR_OFFSET];
    INDEX_FILE.open(indexFilename, ios::binary);

    findQuery(stringQuery);
    outPutQuery();
    INPUT_FILE.close();
    INDEX_FILE.close();
    return 0;
}
