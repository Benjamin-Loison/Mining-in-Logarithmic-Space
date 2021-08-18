#include <iostream>
#include <string>
#include <vector>
#include <dirent.h>
#include <sstream>
#include <fstream>
#include <bitset>
#include <map>
#include <algorithm>
#include <ctime>
#include <sys/time.h>
using namespace std;

#define BLOCK pair<unsigned short, unsigned int>
#define UNSIGNED_INT_MAX 4294967295
#define UNSIGNED_SHORT_MAX 65535

//#define ALL_DIRECTLY

template<typename T>
string convertNbToStr(const T& number)
{
    ostringstream convert;
    convert << number;
    return convert.str();
}

void print(string s, bool withDelta = true);
vector<string> listFiles(string folder), getFileContent(string path), split(string s, string delimiter = " ");
bool endsWith(const string&, const string&), startsWith(string subject, string test), writeFile(string filePath, string option, string toWrite);
string toBin(string hex);
unsigned short getBinZeroLeading(string hex);
int convertStrToInt(string str);
long long convertStrToLongLong(string str); // long is enough
unsigned long long getMillis();
unsigned long long lastDateTime = 0;

string blocksFolder = "/mnt/f/Bitcoin/Data/blocks/";

char pathSeparator = '/';
string pathSeparatorStr = "/";
vector<string> files;

char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

unsigned int filesSize, filesTreated = 0;
string filesSizeStr;

// a file pointer is: pair<unsigned short, unsigned int> (unsigned short is the number of the blk file, unsigned int is the position of the block within the file)
// the timestamp of a given block is unique across all blocks, we could use it as an unique identifier however instead of current system we would lost the piece of information of where the block is among all blk files (and where within a blk file) if we need extra data
map<BLOCK, unsigned short> blocksZeros;
map<BLOCK, unsigned int> blocksTimestamps;
map<unsigned int, BLOCK> timestampsBlocks;
unsigned short k = 6, m = 3 * k;
vector<BLOCK> getBlocksFromBLKs(), Compress(vector<BLOCK> Ci, bool verbose = true), timestampSort(vector<BLOCK>* blocks), intersection(vector<BLOCK> blocks0, vector<BLOCK> blocks1), getBlocksFromB(vector<BLOCK>* blocks, BLOCK b, bool before = false), getOnlyDistinctBlocks(vector<BLOCK>* blocks);
bool timestampSorted(vector<BLOCK>* blocks);
unsigned int getBlocksIndexFromB(vector<BLOCK>* blocks, BLOCK b), getBlocksIndexFromBDumb(vector<BLOCK>* blocks, BLOCK b);
#ifndef ALL_DIRECTLY
	vector<pair<unsigned int, unsigned short>> dataOneByOne;
	vector<unsigned short> difficulties;
	map<unsigned short, vector<BLOCK>> CStarArrow;
#endif

int main()
{
	files = listFiles(blocksFolder);
	filesSize = files.size();
	vector<string> newFiles;
	for(unsigned int filesIndex = 0; filesIndex < filesSize; filesIndex++)
    {
		string file = files[filesIndex];
		if(startsWith(file, "blk") && endsWith(file, ".dat"))
		{
			newFiles.push_back(file);
		}
	}
	files = newFiles;
	filesSize = files.size();
	filesSizeStr = convertNbToStr(filesSize);

	vector<BLOCK> C = getBlocksFromBLKs(); // CHRONOLOGICALLY ORDERED ! - checked
	unsigned int CSize = C.size();
	print("CSize: " + convertNbToStr(CSize));

	#ifdef ALL_DIRECTLY
		vector<BLOCK> piX = Compress(C);
	#else
		// blocks one by on (using theorem 3: Compress(Cb) = Compress(pib))
		vector<BLOCK> piX;
		string CSizeStr = convertNbToStr(CSize);
		unsigned long long time = getMillis();
		unsigned int CLastIndex = 0;
		for(unsigned int CIndex = 0; CIndex < CSize; CIndex++)
		{
			unsigned long long currentTime = getMillis();
			bool isAfterAPeriod = currentTime > time + 10000;
			if(isAfterAPeriod)
			{
				print("", false);
				print(convertNbToStr(CIndex) + " / " + CSizeStr);
				print(convertNbToStr(CIndex - CLastIndex) + " blocks per period !");
				CLastIndex = CIndex;
				time = currentTime;
			}

			BLOCK b = C[CIndex];
			piX.push_back(b);
		
			pair<unsigned int, unsigned short> timestampZeros = dataOneByOne[CIndex];
			unsigned int timestamp = timestampZeros.first;
			unsigned short zeros = timestampZeros.second;
			blocksZeros[b] = zeros;
			blocksTimestamps[b] = timestamp;
			timestampsBlocks[timestamp] = b;
			if(find(difficulties.begin(), difficulties.end(), zeros) == difficulties.end())
			{
				difficulties.push_back(zeros);
				vector<BLOCK> blocks;
            	CStarArrow[zeros] = blocks;
			}

			piX = Compress(piX, isAfterAPeriod);
		}
	#endif
			
	print("compressed");
	unsigned int piXSize = piX.size();
	print("piXSize: " + convertNbToStr(piXSize));
	
	// 2 066 (more than expected because we don't just take 2 * m for each level), 41 blocks weren't expected, so without them we got 2 025 which is so our lower bound because we didn't go to 0 because it doesn't really make sens
	// ℓ = 86
	// C*↑^ℓ = 75
	// expected to be: between C*↑^ℓ + k = 81 and C*↑^ℓ + k + 2 * m * 86 = 3 177
	// ideally (if take T_0 not equal to zero but 32), C*↑^ℓ + k + 2 * m * 54 = 2 025
	// min mu = 32

	string piXStr = "";
	for(unsigned int piXIndex = 0; piXIndex < piXSize; piXIndex++)
	{
		BLOCK piXEl = piX[piXIndex];
		unsigned short blockNumber = piXEl.first;
		unsigned int blockPosition = piXEl.second;
		piXStr += convertNbToStr(blockNumber) + " " + convertNbToStr(blockPosition);
		if(piXIndex < piXSize - 1)
		{
			piXStr += "\n";
		}
	}
	writeFile("piX.txt", "w", piXStr);

	return 0;
}

// storing in a single file: timestamp, blk number, position in file, hash should be cool
// make sure that the hash is the current one not the previous one - checked
// use a saved file likewise if following algorithm has problems we don't have to rerun read all blk files
vector<BLOCK> getBlocksFromBLKs()
{
	vector<BLOCK> blocks;
    vector<string> lines = getFileContent("dataSortedAndShifted.txt"); // checked also another way that this file is perfectly formated
    unsigned int linesSize = lines.size();
    short lastBlockNumber = 0, maxDeltaAbs = 0;
    for(unsigned int linesIndex = 0; linesIndex < linesSize; linesIndex++)
    {
        string line = lines[linesIndex];
        vector<string> lineParts = split(line);
        unsigned short linePartsSize = lineParts.size();
        if(linePartsSize != 4)
            print("format problem");
        string hash = lineParts[3];
		unsigned int timestamp = convertStrToInt(lineParts[0]); // could use long
        short blockNumber = convertStrToInt(lineParts[1]), delta = blockNumber - lastBlockNumber, deltaAbs = abs(delta);
        if(deltaAbs > maxDeltaAbs)
            maxDeltaAbs = deltaAbs;
		lastBlockNumber = blockNumber;
        unsigned long blockPosition = convertStrToLongLong(lineParts[2]);
		BLOCK block = make_pair(blockNumber, blockPosition);
        blocks.push_back(block);
		unsigned short blockZeros = getBinZeroLeading(hash);
		#ifdef ALL_DIRECTLY
			blocksZeros[block] = blockZeros;
			blocksTimestamps[block] = timestamp;
			timestampsBlocks[timestamp] = block;
		#else
			dataOneByOne.push_back(make_pair(timestamp, blockZeros));
		#endif
    }
	#ifdef ALL_DIRECTLY
		print("blocksZeros.size(): " + convertNbToStr(blocksZeros.size()));
	#endif
	return blocks;
}

unsigned short maxHey = 0;

// may have to optimize using pointers or global variables
tuple<map<unsigned short, vector<BLOCK>>, unsigned short, vector<BLOCK>> Dissolve(vector<BLOCK> C, bool verbose = true)
{
    unsigned int CSize = C.size();
    unsigned short l = 0;

    // C* ← C[:-k]
    vector<BLOCK> CStar;
    if(CSize >= k)
    {
        vector<BLOCK> extracted(C.begin(), C.end() - k);
        CStar = extracted;
    }
    else
        CStar = C;
	// D ← Ø
	map<unsigned short, vector<BLOCK>> D;

    if(CStar.size() >= 2 * m)
    {
        // ℓ ← max{µ: |C*↑^µ|≥2m}
        map<unsigned short, unsigned int> blocksPerZeros;
        for(map<BLOCK, unsigned short>::iterator it = blocksZeros.begin(); it != blocksZeros.end(); it++)
        {
            unsigned short blockZeros = it->second;
            if(blocksPerZeros.find(blockZeros) != blocksPerZeros.end())
                blocksPerZeros[blockZeros]++;
            else
                blocksPerZeros[blockZeros] = 1;
        }
		if(verbose)
		{
			print("blocksPerZeros.size(): " + convertNbToStr(blocksPerZeros.size()));
			print("blockZeros computed !");
		}
        for(map<unsigned short, unsigned int>::iterator it = blocksPerZeros.begin(); it != blocksPerZeros.end(); it++)
        {
            unsigned short mu = it->first, blocksNumber = it->second;
            if(mu > l && blocksNumber >= 2 * m)
                l = mu;
        }
		if(verbose)
			print("l (" + convertNbToStr(l) + ") found !");
        // could make a function taking n and returning C*↑^n - see CStarArrow
        // D[ℓ] ← C*↑^ℓ
		#ifdef ALL_DIRECTLY
			map<unsigned short, vector<BLOCK>> CStarArrow;
		#endif

		// d is the difficulty range: 54
        // n is the number of blocks: 687 104

		#ifdef ALL_DIRECTLY
			for(map<BLOCK, unsigned short>::iterator it = blocksZeros.begin(); it != blocksZeros.end(); it++)
        	{
            	BLOCK block = it->first;
            	unsigned short blockZeros = it->second;
            	if(CStarArrow.find(blockZeros) == CStarArrow.end())
            	{
                	vector<BLOCK> blocks;
                	blocks.push_back(block);
                	CStarArrow[blockZeros] = blocks;
            	}
            	else
                	CStarArrow[blockZeros].push_back(block);
        	}

			for(map<unsigned short, vector<BLOCK>>::iterator it = CStarArrow.begin(); it != CStarArrow.end(); it++)
        	{
            	unsigned short blockZeros = it->first;
            	vector<BLOCK> blocks = it->second;
            	unsigned int blocksSize = blocks.size();
            	vector<BLOCK> blocksSorted = timestampSort(&blocks);
            	CStarArrow[blockZeros] = blocksSorted;
        	}
		#else
			map<unsigned int, BLOCK>::reverse_iterator it = timestampsBlocks.rbegin();
			unsigned int timestamp = it->first;
			BLOCK block = it->second;
			unsigned short blockZeros = blocksZeros[block];
        	CStarArrow[blockZeros].push_back(block);
		#endif

		D[l] = CStarArrow[l];
		if(verbose)
			print("D[l].size(): " + convertNbToStr(D[l].size()));

		unsigned short heyCounter = 0;
        for(short mu = l - 1; mu >= /*0*/32; mu--) // does it even make sens to go to 0 ?
        {
            // b ← C*↑^(µ+1)[-m]
            // D[µ] ← C*↑^µ[-2m:]UC*↑^µ{b:}
			vector<BLOCK> mu2m,
				          *CStarMu = &CStarArrow[mu],
						  *CStarMuPlus1 = &CStarArrow[mu + 1];
			if(CStarMu->size() >= 2 * m) // should always be the case - maybe not at initialization
			{
				vector<BLOCK> extracted(CStarMu->end() - 2 * m, CStarMu->end());
				mu2m = extracted;
			}
			else
			{
				//print("why me ?"); // is called when from initialization
				mu2m = *CStarMu;
			}
			// could use integers here
			D[mu].insert(D[mu].end(), mu2m.begin(), mu2m.end()); // hard to optimize by keeping D because D can change at all levels at every round...
			unsigned int CStarMuPlus1Size = CStarMuPlus1->size();
			BLOCK b = CStarMuPlus1->at(CStarMuPlus1Size >= m ? CStarMuPlus1Size - m : 0);
			//unsigned long long time = getMillis();
			unsigned int CStarMuFromBIndexBisBis = getBlocksIndexFromBDumb(CStarMu, b);
			//vector<BLOCK> CStarMuFromB = getBlocksFromB(CStarMu, b);
			//unsigned int CStarMuFromBIndexBis = CStarMu->size() - CStarMuFromB.size();
			/*unsigned long long currentTime = getMillis(), deltaA = currentTime - time;
			//if(deltaA > 1)
			//	print("a: " + convertNbToStr(deltaA));
			unsigned int CStarMuFromBIndex = getBlocksIndexFromB(CStarMu, b);
			unsigned long long deltaB = getMillis() - currentTime;
			//if(deltaB > 1)
			if(deltaA > 1)
			{
				print("a: " + convertNbToStr(deltaA));
				print("b: " + convertNbToStr(deltaB));
			}*/
			/*if(CStarMuFromBIndexBis != CStarMuFromBIndex) // different before block 22 803
			{
				print("CStarMuFromBIndexes: " + convertNbToStr(CStarMuFromBIndexBis) + " " + convertNbToStr(CStarMuFromBIndex));
				//exit(1);
			}*/
			//if(CStarMuFromBIndexBisBis != CStarMuFromBIndexBis)
			//	print("CStarMuFromBIndexes: " + convertNbToStr(CStarMuFromBIndexBisBis) + " " + convertNbToStr(CStarMuFromBIndexBis));

			//D[mu].insert(D[mu].end(), CStarMuFromB.begin(), CStarMuFromB.end());
			//print("add n blocks: " + convertNbToStr(CStarMu->size() - CStarMuFromBIndexBisBis)); // should be 0 much of time because notExpected blocks was low likewise we could not sort if original data is sorted (which is the case) ! - hum may not be 0 much of time because it's duplicated elements from the union
			//if(CStarMu->begin() + CStarMuFromBIndexBisBis < CStarMu->end() - 2 * m) // TODO: il doit y avoir quelque chose à faire dans ce goût là en cancel l'insert et le sort si ce n'est pas le cas
			if(CStarMu->size() - CStarMuFromBIndexBisBis > 2 * m)
			{
				//print("C.size() " +  convertNbToStr(C.size()) + " hey"); // called three times when ALL_DIRECTLY
				heyCounter++;
			}
			D[mu].insert(D[mu].end(), CStarMu->begin() + CStarMuFromBIndexBisBis, CStarMu->end());
			//if(timestampSorted(&D[mu])) print("already sorted !"); // not called after begining
			D[mu] = timestampSort(&D[mu]);
        }
		if(heyCounter > maxHey)
		{
			maxHey = heyCounter;
			print("maxHey: " + convertNbToStr(maxHey) + " " + convertNbToStr(D.size()));
		}
    }
    else
    {
        // D[0] ← C*
        D[0] = CStar;
    }

    // χ ← C[-k:]
    vector<BLOCK> X;
    if(CSize >= k)
    {
        vector<BLOCK> extracted(C.end() - k, C.end());
        X = extracted;
    }
    else
        X = C;

	// return (D, ℓ, χ)
    return make_tuple(D, l, X);
}

vector<BLOCK> Compress(vector<BLOCK> C, bool verbose)
{
	// (D, ℓ, χ) ← Dissolve(C)
	tuple<map<unsigned short, vector<BLOCK>>, unsigned short, vector<BLOCK>> DlX = Dissolve(C, verbose);
	if(verbose)
		print("dissolved");
	map<unsigned short, vector<BLOCK>> D = get<0>(DlX);
	unsigned short l = get<1>(DlX);
	vector<BLOCK> X = get<2>(DlX);
	// do we care about chronologically ordered here ? - doing it shouldn't be a problem and can solve some
	// π ← U_{µ=0}^l D[µ]
	vector<BLOCK> pi;
	for(unsigned short mu = 0; mu <= l; mu++)
	{
		vector<BLOCK> Dmu = D[mu];
		pi.insert(pi.end(), D[mu].begin(), D[mu].end());
	}
	if(verbose)
		print("pi.size(): " + convertNbToStr(pi.size()));
	// return πχ
	vector<BLOCK> piX = pi;
	piX.insert(piX.end(), X.begin(), X.end());
	return piX;
}

vector<BLOCK> maxvalid(vector<BLOCK> pi, vector<BLOCK> piBis)
{
	// TODO
	// if Π is not valid
	// return Π′
	// return piBis;
	
	// if Π′ is not valid
	// return Π
	// return pi;
	
	// (χ, ℓ, D) ← Dissolve(Π)
	// (χ′, ℓ′, D′) ← Dissolve(Π′)
	tuple<map<unsigned short, vector<BLOCK>>, unsigned short, vector<BLOCK>> DlX = Dissolve(pi), DlXBis = Dissolve(piBis);
	map<unsigned short, vector<BLOCK>> D = get<0>(DlX), DBis = get<0>(DlXBis);
	unsigned short l = get<1>(DlX), lBis = get<1>(DlXBis);
	// M ← {μ \in N: D[μ] ∩ D′[μ]̸ ≠ Ø}
	unsigned short lowestMu = UNSIGNED_SHORT_MAX;
	vector<unsigned short> M;
	for(map<unsigned short, vector<BLOCK>>::iterator it = D.begin(); it != D.end(); it++)
	{
		unsigned short mu = it->first;
		if(DBis.find(mu) != DBis.end())
		{
			M.push_back(mu);
			if(mu < lowestMu)
				lowestMu = mu;
		}
	}
	
	// if M = Ø
	if(M.empty())
		// if l′ > l return Π′ otherwise return Π
		return lBis > l ? piBis : pi;

	// µ ← min M
	unsigned short mu = lowestMu;
	// b ← (D[µ] ∩ D′[µ])[-1]
	// if |D′[µ]{b:}| > |D[µ]{b:}| return Π′ otherwise return Π
	vector<BLOCK> DMu = D[mu], DBisMu = DBis[mu], inter = intersection(DMu, DBisMu);
	BLOCK b = inter[inter.size() - 1];
	unsigned int bTimestamp = blocksTimestamps[b];
	vector<BLOCK> DMuFromB = getBlocksFromB(&DMu, b), DBisMuFromB = getBlocksFromB(&DBisMu, b);
	return DBisMuFromB.size() > DMuFromB.size() ? piBis : pi;
}

vector<BLOCK> uponNewBlockReceived(vector<BLOCK> pi, vector<BLOCK> XBis)
{
	// χ ← Π[-k:]
	unsigned int piSize = pi.size();
	vector<BLOCK> X, subPi;
	if(piSize >= k) // could make a function for such a purpose
	{
		vector<BLOCK> extracted(pi.end() - k, pi.end());
		X = extracted;
	}
	else
		X = pi;

	// π ← Π[:-k]
	if(piSize >= k)
	{
		vector<BLOCK> extracted(pi.begin(), pi.end() - k);
		subPi = extracted;
	}
	// no else logically it seems
	
	// if χ′ is a chain and χ′[0] \in χ
	if(/*check if χ′ is a chain*/find(X.begin(), X.end(), XBis[0]) != X.end())
	{
		// b ← (χ ∩ χ′)[-1]
		vector<BLOCK> inter = intersection(X, XBis);
		BLOCK b = inter[inter.size() - 1];
		// if |χ′{b:}| > |χ{b:}|
		vector<BLOCK> XBisFromB = getBlocksFromB(&XBis, b);
		if(XBisFromB.size() > getBlocksFromB(&X, b).size())
		{
			// Validate χ′ state transitions starting from b
			// todo
			// Π ← Compress(π χ{:b} χ′{b:})
			vector<BLOCK> XBeforeB = getBlocksFromB(&X, b, true);
			subPi.insert(subPi.end(), XBeforeB.begin(), XBeforeB.end());
			subPi.insert(subPi.end(), XBisFromB.begin(), XBisFromB.end());
			pi = Compress(subPi);
			// BROADCAST(Π)
			
		}
	}

	return pi;
}

vector<BLOCK> getOnlyDistinctBlocks(vector<BLOCK>* blocks)
{
	vector<BLOCK> distinctBlocks;
	unsigned int blocksSize = blocks->size();
	for(unsigned int blocksIndex = 0; blocksIndex < blocksSize; blocksIndex++)
	{
		BLOCK block = blocks->at(blocksIndex);
		if(find(distinctBlocks.begin(), distinctBlocks.end(), block) == distinctBlocks.end())
			distinctBlocks.push_back(block);
	}
	return distinctBlocks;
}

unsigned int getBlocksIndexFromBAux(vector<BLOCK>* blocks, unsigned int bTimestamp, unsigned int lower, unsigned int upper/*, unsigned short calls*/) // O(log_2(n) * log_2(n) = 19.39 * 19.39 = 375.97)
{
	//print("gIBFBA: " + convertNbToStr(lower) + " " + convertNbToStr(upper));
	int m = (upper - lower + 1) / 2;
	unsigned int timestamp = blocksTimestamps[blocks->at(lower + m)];
	if(lower >= upper)
	{
		/*if(lower + m + 1 < blocks->size())
		{
			m++;
			timestamp = blocksTimestamps[blocks->at(lower + m)];
		}*/
		while(timestamp > bTimestamp)
		{
			m--;
			if(m == -1)
			{
				//print("don't break");
				break;
			}
			timestamp = blocksTimestamps[blocks->at(lower + m)];
			//print("tourne !");
		}
		//print("calls: " + convertNbToStr(calls) + " blocks.size(): " + convertNbToStr(blocks->size())); // seems fair
		print("lower (" + convertNbToStr(lower) + ") m (" + convertNbToStr(m) + ") blocks->size() " + convertNbToStr(blocks->size()) + ")");
		return lower + m + 1;
	}
	if(timestamp > bTimestamp)
	{
		//print("en haut");
		return getBlocksIndexFromBAux(blocks, bTimestamp, lower, upper - m/* - 1*//*, calls + 1*/);
	}
	else
	{
		//print("en bas");
		return getBlocksIndexFromBAux(blocks, bTimestamp, lower + m/* + 1*/, upper/*, calls + 1*/);
	}
}

unsigned int getBlocksIndexFromB(vector<BLOCK>* blocks, BLOCK b)
{
	if(!timestampSorted(blocks)) print("not sorted !");
	unsigned int bTimestamp = blocksTimestamps[b]; // b isn't always in blocks
	return getBlocksIndexFromBAux(blocks, bTimestamp, 0, blocks->size() - 1/*, 1*/);
}

unsigned int getBlocksIndexFromBDumb(vector<BLOCK>* blocks, BLOCK b)
{
	unsigned int bTimestamp = blocksTimestamps[b], blocksSize = blocks->size();
    for(unsigned int blocksIndex = 0; blocksIndex < blocksSize; blocksIndex++)
    {
        BLOCK block = blocks->at(blocksIndex);
        unsigned int timestamp = blocksTimestamps[block];
        if(timestamp >= bTimestamp)
            return blocksIndex;
    }
	//print("blocksSize: " + convertNbToStr(blocksSize));
    return blocksSize/* - 1*/;
}

vector<BLOCK> getBlocksFromB(vector<BLOCK>* blocks, BLOCK b, bool before) // O(n * log_2(n) = 13 148 436) | if sure that chronologically ordered can use dichotomy
{
	//if(!timestampSorted(blocks)) print("not sorted !"); // seems called sometimes after having a good dataset - TODO this is weird should check !
	unsigned int bTimestamp = blocksTimestamps[b], blocksSize = blocks->size();
    vector<BLOCK> blocksFromB;
    for(unsigned int blocksIndex = 0; blocksIndex < blocksSize; blocksIndex++)
    {
    	BLOCK block = blocks->at(blocksIndex);
        unsigned int timestamp = blocksTimestamps[block];
        if((!before && timestamp >= bTimestamp) || (before && timestamp <= bTimestamp)) // > should do the same - may depend on new uses of this function
        	blocksFromB.push_back(block); // likewise we keep sorted on this side
	}
	return blocksFromB;
}

vector<BLOCK> intersection(vector<BLOCK> blocks0, vector<BLOCK> blocks1)
{
	vector<BLOCK> inter;
	unsigned int blocks0Size = blocks0.size();
	for(unsigned int blocks0Index = 0; blocks0Index < blocks0Size; blocks0Index++)
	{
		BLOCK block0 = blocks0[blocks0Index];
		if(find(blocks1.begin(), blocks1.end(), block0) != blocks1.end())
			inter.push_back(block0);
	}
	return inter;
}

vector<BLOCK> timestampSort(vector<BLOCK>* blocks) // doesn't used to be a pointer
{
	vector<BLOCK> blocksSorted;
    unsigned int blocksSize = blocks->size();
	map<unsigned int, BLOCK> timestamps;
	for(unsigned int blocksIndex = 0; blocksIndex < blocksSize; blocksIndex++)
	{
		BLOCK block = blocks->at(blocksIndex);
		unsigned int timestamp = blocksTimestamps[block];
		timestamps[timestamp] = block;
	}
	for(map<unsigned int, BLOCK>::iterator it = timestamps.begin(); it != timestamps.end(); it++)
		blocksSorted.push_back(it->second);
	// never got problems so let's comment to optimize
	//if(!timestampSorted(blocksSorted)) print("blocksSorted not sorted !");
	return blocksSorted;
}

bool timestampSorted(vector<BLOCK>* blocks) // doesn't used to be a pointer
{
	unsigned int blocksSize = blocks->size(), lastTimestamp = 0;
	for(unsigned int blocksIndex = 0; blocksIndex < blocksSize; blocksIndex++)
	{
		BLOCK block = blocks->at(blocksIndex);
		unsigned int timestamp = blocksTimestamps[block];
		if(timestamp <= lastTimestamp)
			return false;
		lastTimestamp = timestamp;
	}
	return true;
}

bool writeFile(string filePath, string option, string toWrite)
{
    FILE* file = fopen(filePath.c_str(), option.c_str());
    if(file != NULL)
    {
        fputs(toWrite.c_str(), file);
        fclose(file);
        return true;
    }
    return false;
}

vector<string> getFileContent(string path)
{
	vector<string> vec;
	ifstream infile(path.c_str());
    string line;
    while(getline(infile, line))
    	vec.push_back(line);
    return vec;
}

int convertStrToInt(string str)
{
    int number;
    sscanf(str.c_str(), "%d", &number);
    return number;
}

vector<string> split(string s, string delimiter)
{
    vector<string> toReturn;
    size_t pos = 0;
    while((pos = s.find(delimiter)) != string::npos)
    {
        toReturn.push_back(s.substr(0, pos));
        s.erase(0, pos + delimiter.length());
    }
    toReturn.push_back(s);
    return toReturn;
}

long long convertStrToLongLong(string str)
{
    long long res = stoll(str.c_str());
    return res;
}

const char* hex_char_to_bin(char c)
{
    switch(toupper(c))
    {
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'A': return "1010";
        case 'B': return "1011";
        case 'C': return "1100";
        case 'D': return "1101";
        case 'E': return "1110";
        case 'F': return "1111";
    }
}

string toBin(string hex)
{
    string bin;
    for(unsigned i = 0; i < hex.length(); i++)
       bin += hex_char_to_bin(hex[i]);
    return bin;
}

unsigned short getBinZeroLeading(string hex)
{
	string bin = toBin(hex);
	unsigned short binLength = bin.length(), zeros = 0;
	for(unsigned short binIndex = 0; binIndex < binLength; binIndex++)
	{
		if(bin[binIndex] == '0')
			zeros++;
		else
			break;
	}
	return zeros;
}

string replace(string subject, const string& search, const string& replace = "")
{
    unsigned int s = subject.find(search);
    if(s > subject.length())
        return subject;
    return subject.replace(s, search.length(), replace);
}

bool startsWith(string subject, string test)
{
    return !subject.compare(0, test.size(), test);
}

void listFiles(string direc, vector<string>* str)
{
    DIR* dir;
    struct dirent* ent;
    if((dir = opendir(direc.c_str())) != NULL)
    {
        while((ent = readdir(dir)) != NULL)
        {
            string file = ent->d_name;
            if(!startsWith(file, "."))
            {
                string path = direc;
                if(path[path.length() - 1] != pathSeparator)
                    path += pathSeparator;
                path += file;
                if(file.find_last_of(".") > file.length())
                    listFiles(path, str);
                else
                    str->push_back(path);
            }
        }
        closedir(dir);
    }
}

bool endsWith(string const& value, string const& ending)
{
    if(ending.size() > value.size()) return false;
    return equal(ending.rbegin(), ending.rend(), value.rbegin());
}

vector<string> listFiles(string directory)
{
	vector<string> files;
    listFiles(directory, &files);
	unsigned int filesSize = files.size();
	for(unsigned int filesIndex = 0; filesIndex < filesSize; filesIndex++)
	{
		files[filesIndex] = replace(files[filesIndex], directory);
	}
    return files;
}

void realPrint(string s)
{
	cout << s << endl;
}

unsigned long long getMillis()
{
    struct timeval tp;
    if(gettimeofday(&tp, NULL) == -1) realPrint("gettimeofday failed !");
    return (long long)tp.tv_sec * 1000L + tp.tv_usec / 1000;
}

string getNbZero(double number, unsigned short numberOfDigits = 2, bool atTheEnd = false)
{
    string strNb = convertNbToStr(number);
    for(unsigned short digit = strNb.length(); digit < numberOfDigits; digit++)
        strNb = atTheEnd ? strNb + "0" : "0" + strNb;
    return strNb;
}

string getHoursMinutesSeconds(string hourMinuteSeparator, string minuteSecondSeparator, bool withDelta)
{
	unsigned long long currentTime = getMillis(),
				       delta = lastDateTime == 0 ? 0 : currentTime - lastDateTime;
	lastDateTime = currentTime;
	string deltaStr = "";
	if(withDelta && delta > 1) // let's not say != 0 otherwise "natural second going on" is shown
		deltaStr = " (Δ = " + convertNbToStr(delta) + " ms)";

    time_t t = time(0);
	if(t == ((time_t)-1)) realPrint("time failed !"); // used to be print #recursive
    struct tm* now = localtime(&t);
	if(now == NULL) realPrint("localtime failed !");

    return getNbZero(now->tm_hour) + hourMinuteSeparator + getNbZero(now->tm_min) + minuteSecondSeparator + getNbZero(now->tm_sec) + ":" + convertNbToStr(getMillis() % 1000) + deltaStr;
}

void print(string s, bool withDelta)
{
	cout << getHoursMinutesSeconds(":", ":", withDelta) << ": " << s << endl;
}
