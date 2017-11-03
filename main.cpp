#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <queue>
#include <algorithm>
#include "dem.h"
#include "Node.h"
#include "utils.h"
#include <time.h>
#include <list>
#include <unordered_map>

using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::getline;
using std::fstream;
using std::ifstream;
using std::priority_queue;
using std::binary_function;


typedef std::vector<Node> NodeVector;
typedef std::priority_queue<Node, NodeVector, Node::Greater> PriorityQueue;

//compute stats for a DEM
void calculateStatistics(const CDEM& dem, double* min, double* max, double* mean, double* stdDev)
{
	int width = dem.Get_NX();
	int height = dem.Get_NY();

	int validElements = 0;
	double minValue, maxValue;
	double sum = 0.0;
	double sumSqurVal = 0.0;
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			if (!dem.is_NoData(row, col))
			{
				double value = dem.asFloat(row, col);
				
				if (validElements == 0)
				{
					minValue = maxValue = value;
				}
				validElements++;
				if (minValue > value)
				{
					minValue = value;
				}
				if (maxValue < value)
				{
					maxValue = value;
				}

				sum += value;
				sumSqurVal += (value * value);
			}
		}
	}

	double meanValue = sum / validElements;
	double stdDevValue = sqrt((sumSqurVal / validElements) - (meanValue * meanValue));
	*min = minValue;
	*max = maxValue;
	*mean = meanValue;
	*stdDev = stdDevValue;
}
bool Diff(const char *demA, const char *demB, char *resultPath, GDALDataType type, double nodatavalue)
{
	CDEM originDEM;
	double originDEMgeoTransformEles[6];
	if (!readTIFF(demA, type, originDEM, originDEMgeoTransformEles))
	{
		cout<<"Failed to read tiff file!"<<endl;
		return true;
	}
	CDEM DEMarc;
	double DEMarcgeoTransformEles[6];
	if (!readTIFF(demB, type, DEMarc, DEMarcgeoTransformEles))
	{
		cout<<"Failed to read tiff file!"<<endl;
		return true;
	}
	CDEM* result = diff(originDEM, DEMarc);
	double min, max, mean, stdDev;
	calculateStatistics(*result, &min, &max, &mean, &stdDev);
	if (fabs(min-max)>0.00001f) {
		printf("\n**********************************************\n*************************************************\nDEMs are different. min=%lf, max=%lf\n",min,max);
		return false;
	}
	else {
		printf("\nDEMs are identical.\n");
	}
	delete result;
	return true;
}

int main(int argc, char* argv[])
{
	
	//argc=5;
	//argv[1]="wei-diff";
	//argv[2]="D:\\测试数据\\DEM Data\\原始文件\\Size5000\\grant-3m_5000_RandomFillwei.tif";
	//argv[3]="D:\\测试数据\\DEM Data\\原始文件\\Size5000\\grant-3m_5000_RandomFillzhou.tif";
	//argv[4]="D:\\测试数据\\DEM Data\\原始文件\\Size5000\\grant-3m_5000_RandomFill_testsub.tif";
	if (argc < 5)
	{
		return 1;
	}
	string pathsrc1(argv[2]);
	string pathsrc2(argv[3]);
	string outputFilledPath(argv[4]);
	size_t indexsrc1 = pathsrc1.find(".tif");
	size_t indexsrc2 = pathsrc1.find(".tif");
	if (indexsrc1==string::npos&&indexsrc2==string::npos) {
		cout<<"Input file name should have an extension of '.tif'"<<endl;
		return 1;
	}
	char* method=argv[1];
	string strFolder = pathsrc1.substr(0, indexsrc1);

	if (strcmp(method,"wei-diff")==0)
	{
		bool result=Diff(&pathsrc1[0],&pathsrc2[0],&outputFilledPath[0],GDT_Float32,-9999); 
		if (result)
		{
			cout<<"There is no difference between the two images!"<<endl;
		}
		else{cout<<"There is difference between the two images!!!!!!!!!!!!!!"<<endl;}
	}
	cout<<"------------------------------------------------------------------------------"<<endl;
	return 0;
}

