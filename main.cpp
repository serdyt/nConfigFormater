#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <cfloat>
#include <sstream>
#include <string>
#include <stdlib.h>

using namespace std;

struct destTriplet{
	int offsetX;
	int offsetY;
	int neuronID;

	destTriplet(int offsetX_, int offsetY_, int neuronID_){
		offsetX = offsetX_;
		offsetY = offsetY_;
		neuronID = neuronID_;
	}
};

int coord2ID(int x, int y, int dimX){
	return x + y*dimX;
}

pair <int, int> ID2coord(int ID, int dimX){
	return pair<int, int>(ID % dimX, ID / dimX);
}

int main() {
	ofstream nConfigFile;
	nConfigFile.open("edgeDetection.cfg", fstream::out);

	nConfigFile << "@step=0.0005"<<endl;

	//const int imgWidth = 30;
	//const int imgHeigth = 30;

	vector<vector<int> > data;
	ifstream infile("test.txt");

	while (infile) {
		string s;
		if (!getline(infile, s))
			break;

		istringstream ss(s);
		vector<int> record;

		while (ss) {
			string s;
			if (!getline(ss, s, ','))
				break;
			record.push_back(atoi(s.c_str()));
		}

		data.push_back(record);
	}

	const int imgWidth = data[0].size();
	const int imgHeigth = data.size();

	cout << "imgWidth = " << imgWidth;
	cout << " imgHeigth = " << imgHeigth << endl;

	for (auto a : data){
		for (auto b : a){
			cout << "," << b;
		}
		cout<< endl;
	}

	const int windSize = 3; //nxn
	const int dimX = ceil(imgWidth / windSize);
	const int dimY = ceil(imgHeigth / windSize);

	const int groupW = 3;
	const int groupH = 2;

	const int weightL2 = 20;
	const int weightL1 = 18;

	const float alpha = 0.004524886877828055;
	const int IexCoeff = 20;

	//TODO : recalculate this
	const float wieghtMatrix[4][windSize*windSize] =
	{	{ .34, .35, .34,
		    0,   0,   0,
		 -.34,-.35,-.34},

		{-.34,-.35,-.34,
		    0,   0,   0,
		  .34, .35, .34},

		{ .34,   0,-.34,
		  .35,   0,-.35,
		  .34,   0,-.34},

		{-.34,   0, .34,
		 -.35,   0, .35,
		 -.34,   0, .34}
	};

	const destTriplet destOffset[windSize*windSize][4] =
	{
	 {destTriplet(1, 0,0), destTriplet(1, 0,1), destTriplet(1, 0,2), destTriplet(1, 0,3)},
	 {destTriplet(1, 0,4), destTriplet(1, 0,5), destTriplet(1, 0,6), destTriplet(1, 0,7)},
	 {destTriplet(1, 0,8), destTriplet(2, 0,0), destTriplet(2, 0,1), destTriplet(2, 0,2)},
	 {destTriplet(2, 0,3), destTriplet(2, 0,4), destTriplet(2, 0,5), destTriplet(2, 0,6)},
	 {destTriplet(2, 0,7), destTriplet(2, 0,8), destTriplet(0,-1,0), destTriplet(0,-1,1)},
	 {destTriplet(0,-1,2), destTriplet(0,-1,3), destTriplet(0,-1,4), destTriplet(0,-1,5)},
	 {destTriplet(0,-1,6), destTriplet(0,-1,7), destTriplet(0,-1,8), destTriplet(2,-1,0)},
	 {destTriplet(2,-1,1), destTriplet(2,-1,2), destTriplet(2,-1,3), destTriplet(2,-1,4)},
	 {destTriplet(2,-1,5), destTriplet(2,-1,6), destTriplet(2,-1,7), destTriplet(2,-1,8)},
	};


	//[cur][dest]
	const int windowSearch[windSize*windSize][windSize*windSize] =
	{
//			{0,5,3,7,8,6,1,2,8},//cur = 0
//			{3,4,5,6,7,8,0,1,2},
//			{5,3,4,8,6,7,2,0,1},
//			{1,2,0,4,5,3,7,8,6},
//			{0,1,2,3,4,5,6,7,8},
//			{2,0,1,5,3,4,8,6,7},
//			{7,8,6,1,2,0,4,5,3},
//			{6,7,8,0,1,2,3,4,5},
//			{8,6,7,2,0,1,5,3,4}

						{4,3,5,1,0,2,7,6,8},//cur = 0
						{5,4,3,2,1,0,8,7,6},
						{3,5,4,0,2,1,6,8,7},
						{7,6,8,4,3,5,1,0,2},
						{8,7,6,5,4,3,2,1,0},
						{6,8,7,3,5,4,0,2,1},
						{1,0,2,7,6,8,4,3,4},
						{2,1,0,8,7,6,5,4,3},
						{0,2,1,6,8,7,3,5,4}
	};

	for (int yNeurL1 = 0; yNeurL1 < imgWidth; yNeurL1++){
		for (int xNeurL1 = 0; xNeurL1 < imgHeigth; xNeurL1 ++){
			int curGroupX = xNeurL1 / windSize;
			int curGroupY = yNeurL1 / windSize;

			int curClusterX = curGroupX * groupW;
			int curClusterY = curGroupY * groupH + 1;
			int curClusterID = coord2ID(curClusterX, curClusterY, dimX * groupW);
			//int neuronID = yNeurL1 % windSize + (xNeurL1 % windSize) * windSize;

			int curNeuronInItsGroupX = xNeurL1 - curGroupX * windSize;
			int curNeuronInItsGroupY = yNeurL1 - curGroupY * windSize;
			int curNeuronInItsGroupID = coord2ID(curNeuronInItsGroupX, curNeuronInItsGroupY, windSize);
			nConfigFile<<"#"<<curClusterID<<","<<curNeuronInItsGroupID;
			nConfigFile << ",V_th=-55,V_reset=-70,E=-70,tau=1000000";
			nConfigFile << ",Iex="<<(data[xNeurL1][yNeurL1]* alpha + 0.3) * IexCoeff;
			nConfigFile<<endl;

			int n = 0;
			for (int yOffset = -1; yOffset <= 1; yOffset++) {
				for (int xOffset = -1; xOffset <= 1; xOffset++) {
					int destNeuronX = xNeurL1 + xOffset;
					int destNeuronY = yNeurL1 + yOffset;

					if (destNeuronX < 1 || destNeuronX > (imgWidth-2) || destNeuronY < 1 || destNeuronY > (imgHeigth-2) ){
						n++;
						continue;
					}

					int destGroupX = destNeuronX / windSize;
					int destGroupY = destNeuronY / windSize;

					int destClusterX = destGroupX * groupW;
					int destClusterY = destGroupY * groupH + 1;

					int destNeuronInItsGroupX = destNeuronX - destGroupX * windSize;
					int destNeuronInItsGroupY = destNeuronY - destGroupY * windSize;
					int destNeuronInItsGroupID = coord2ID(destNeuronInItsGroupX, destNeuronInItsGroupY, windSize);

//					if (xNeurL1 == 6 && yNeurL1 == 5){
//						nConfigFile<<"% curNeuronInItsGroupID " << curNeuronInItsGroupID
//								<< " xNeurL1 " << xNeurL1 << " yNeurL1 " << yNeurL1
//								<< " curGroupX " << curGroupX << " curGroupY " << curGroupY
//								<< " xOffset " << xOffset << " yOffset " << yOffset
//								<< " destNeuronInItsGroupID " << destNeuronInItsGroupID
//								<< endl;
//					}

					for (int j = 0; j < 4; j++){
						int destClusterID = coord2ID(destClusterX + destOffset[destNeuronInItsGroupID][j].offsetX,
													 destClusterY + destOffset[destNeuronInItsGroupID][j].offsetY, dimX * groupW);
						nConfigFile<< ">" << destClusterID << "," << destOffset[destNeuronInItsGroupID][j].neuronID << ",";
						//nConfigFile<< wieghtMatrix[j][windowSearch[curNeuronInItsGroupID][destNeuronInItsGroupID]] * weightL1 << endl;
						nConfigFile<< wieghtMatrix[j][n] * weightL1 << endl;
					}
					n++;
				}
			}
		}
	}

	//connect second layer to output
	for (int groupY = 0; groupY < dimY; groupY++){
		for (int groupX = 0; groupX < dimX; groupX ++){
			int neuronCounter = 0;
			for (int clusterY = groupY * groupH + 1; clusterY >= groupY * groupH; clusterY--){
				for (int clusterX = groupX * groupW; clusterX < (groupX + 1) * groupW; clusterX++){
					int curClusterID = clusterX + clusterY * groupW * dimX;
					if (clusterX == groupX * groupW && clusterY == groupY * groupH + 1) continue;
					if ( clusterX == groupX * groupW + 1 && clusterY == groupY * groupH) {
						for (int neuronID = 0; neuronID < windSize * windSize; neuronID++){
							nConfigFile << "#" << curClusterID << "," << neuronID << ",V_th=-55,V_reset=-70,E=-70,tau=5" <<  endl;
						}
					}
					else {
						for (int neuronID = 0; neuronID < windSize * windSize; neuronID++) {
							int destClusterID = groupX * groupW + 1 + groupY * groupH * dimX * groupW;
							int destNeuronID = neuronCounter / 4;
							nConfigFile << "#" << curClusterID << "," << neuronID << ",V_th=-55,V_reset=-70,E=-70,tau=5" << endl;
							nConfigFile << ">" << destClusterID << "," << destNeuronID << "," << weightL2 << endl;
							neuronCounter++;
						}
					}
				}
			}
		}
	}

	nConfigFile.close();
}
