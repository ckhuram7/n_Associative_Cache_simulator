#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;


vector<string> rwVec;
vector<int> addressVec;

bool cacheSimulate(int cacheSize, int blockSize, int setAss,  ofstream& myout);

int main(int argc,char *argv[])
{
  string filename = argv[1];

	string line, rw, addr;
	vector<string> addrVec;
	int numInst=0;

   /*cout << "input trace file name (without the .trace extension)\n";
	cin >> filename;*/

	ifstream infile(filename+".trace");
	if(infile.is_open()){
        while(!infile.eof()){
            getline(infile,line);

			if(line.empty() == true)
			{
				continue;
			}

			rw = line.substr(0,1);
			addr = line.substr(2,line.size());
			rwVec.push_back(rw);
			addrVec.push_back(addr);
			numInst++;
        }
        infile.close();
    }


	//convert hex to decimal
	int address;
	for(int i =0; i < addrVec.size(); i++)
	{
		stringstream ss;
		ss << hex << addrVec[i].substr(2,addrVec[i].size());
		ss >> address;
		//cout << address << endl;
		addressVec.push_back(address);
	}


	int cacheSize[] = {1024, 4096, 16384, 65536};//131072
	int blockSize[] = {8, 16, 32, 64};
	int associativity[] = {1, 2, 4, 0};

	ofstream outfile;
	outfile.open (filename+".output");


	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			for(int k = 0; k < 4; k++)
			{
				if(k == 3)
				{
					associativity[k] = cacheSize[i] / blockSize[j];
				}
				cacheSimulate(cacheSize[i],blockSize[j],associativity[k], outfile); 
			}
		}
	}

	outfile.close();
    return 0;
}


bool cacheSimulate(int cacheSize, int blockSize, int associativity, ofstream& myout){

	int numBlocks = cacheSize/blockSize;
	int numSets = cacheSize / (associativity * blockSize);
	bool found;
	//cout << "numBlocks " << numBlocks << ", numWays (assoc): " << associativity << ", numSets: " << numSets << endl;

	vector< vector<int> > cache(numSets, vector<int>(associativity));
	vector< vector<int> > recentTagOrder(numSets, vector<int>(associativity));

	for(int i = 0; i < numSets; i++)
	{
		for(int j = 0; j < associativity; j++)
		{
			cache[i][j]=-1;
		}
	}
	for(int i = 0; i < numSets; i++)
	{
		for(int j = 0; j < associativity; j++)
		{
			recentTagOrder[i][j] = j;
		}
	}

	int set, tag, pos;
	int hit = 0;
	int inst = 0;
	int memToCache = 0;
	int cacheToMem = 0;

	for(int run = 0; run < addressVec.size(); run++)
	{
		set = (addressVec[run] / blockSize) % numSets;
		tag = addressVec[run] / (blockSize * numSets); 

		//cout << " tag: " << tag << " set: " << set << endl;
		inst++;

		found = false;
		for(int i = 0; i < associativity; i++)
		{
			if(cache[set][i] == tag)
			{
				found = true;
				pos = i;
			}
		}
		if(found)
		{
			hit++;

			int tempPos= 0;
			for(int i = 0; i < associativity; i++)
			{
				if(recentTagOrder[set][i] == pos){
					tempPos = i;
				}
			}
			for(int i = tempPos; i < associativity-1; i++){
				recentTagOrder[set][i] = recentTagOrder[set][i+1]; 
			}
			recentTagOrder[set][associativity-1] = pos;
		}
		else
		{

			int temp = recentTagOrder[set][0];
			cache[set][temp] = tag;

			int tempPos= 0;
			for(int i = 0; i < associativity; i++)
			{
				if(recentTagOrder[set][i] == temp){
					tempPos = i;
				}
			}
			for(int i = tempPos; i < associativity-1; i++){
				recentTagOrder[set][i] = recentTagOrder[set][i+1]; 
			}
			recentTagOrder[set][associativity-1] = temp;


			if(rwVec[run] == "r" || rwVec[run] == "R")
			{
				memToCache= memToCache + blockSize;
			}
			else if(rwVec[run] == "w" || rwVec[run] == "W")
			{
				cacheToMem = cacheToMem + blockSize;
			}

		}
	}	


	string mapping = "";
	if(associativity == 1)
	{
		mapping = "DM";
	}else if(associativity == 2)
	{
		mapping = "2W";
	}else if(associativity == 4)
	{
		mapping = "4W";
	}else
	{
		mapping = "FA";
	}

	float hitrate = float(hit)/float(inst);
	myout.precision(2);
	myout << fixed << cacheSize << "\t" << blockSize << "\t" << mapping << "\t" << hitrate << "\t" << memToCache + cacheToMem << "\t" << cacheToMem << "\t" << associativity << endl;


	return true;
}
