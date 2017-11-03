#include "utils.h"
#include "gdal_priv.h"
#include "dem.h"
#include <string>
#include <iostream>
using namespace std;

//create a new GeoTIFF file
bool  CreateGeoTIFF(char* path,int height, int width,void* pData, GDALDataType type, double* geoTransformArray6Eles,
					double* min, double* max, double* mean, double* stdDev, double nodatavalue)
{
    GDALDataset *poDataset;   
    GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
 
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    char **papszOptions = NULL;
    poDataset = poDriver->Create(path,width, height, 1, type, 
                                papszOptions );
	

	if (geoTransformArray6Eles != NULL)
		poDataset->SetGeoTransform(geoTransformArray6Eles);


	GDALRasterBand* poBand;
	poBand= poDataset->GetRasterBand(1);
	
	poBand->SetNoDataValue(nodatavalue);

	if (min != NULL && max != NULL && mean != NULL && stdDev != NULL)
	{
		poBand->SetStatistics(*min, *max, *mean, *stdDev);
	}
	poBand->RasterIO( GF_Write, 0, 0, width, height, 
                      pData, width, height, type, 0, 0 );    

	GDALClose( (GDALDatasetH) poDataset );

	return true;
}
//read a DEM GeoTIFF file 
bool readTIFF(const char* path, GDALDataType type, CDEM& dem, double* geoTransformArray6Eles)
{
	GDALDataset *poDataset;   
    GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
	poDataset = (GDALDataset* )GDALOpen(path, GA_ReadOnly);
	if (poDataset == NULL)
	{
		printf("Failed to read the GeoTIFF file\n");
		return false;
	}

	GDALRasterBand* poBand;
	poBand = poDataset->GetRasterBand(1);
	GDALDataType dataType = poBand->GetRasterDataType();
	if (dataType != type)
	{
		return false;
	}
	if (geoTransformArray6Eles == NULL)
	{
		printf("Transformation parameters can not be NULL\n");
		return false;
	}

	memset(geoTransformArray6Eles, 0, 6);
	poDataset->GetGeoTransform(geoTransformArray6Eles);

	dem.SetWidth(poBand->GetXSize());
	dem.SetHeight(poBand->GetYSize());
	
	if (!dem.Allocate()) return false;

	poBand->RasterIO(GF_Read, 0, 0, dem.Get_NX(), dem.Get_NY(), 
		(void *)dem.getDEMdata(), dem.Get_NX(), dem.Get_NY(), dataType, 0, 0);

	GDALClose((GDALDatasetH)poDataset);
	return true;
}
/*
*	neighbor index
*	5  6  7
*	4     0
*	3  2  1
*/
int	ix[8]	= { 0, 1, 1, 1, 0,-1,-1,-1 };
int	iy[8]	= { 1, 1, 0,-1,-1,-1, 0, 1 };

void setNoData(unsigned char* data, int length, unsigned char noDataValue)
{
	if (data == NULL || length == 0)
	{
		return;
	}

	for (int i = 0; i < length; i++)
	{
		data[i] = noDataValue;
	}
}
void setNoData(float* data, int length, float noDataValue)
{
	for (int i = 0; i < length; i++)
	{
		data[i] = noDataValue;
	}
}


const unsigned char value[8] = {128, 64, 32, 16, 8, 4, 2, 1};

CDEM* diff(CDEM& demA, CDEM& demB)
{
	CDEM* pRestultDEM = new CDEM();

	int widthA = demA.Get_NX();
	int heightA = demA.Get_NY();
	int widthB = demB.Get_NX();
	int heightB = demB.Get_NY();

	//widthA== widthB??heightA==heightB??
	if (widthA != widthB || heightA != heightB)
	{
		cout<<"Resolution is different!"<<endl;
		return NULL;
	}

	pRestultDEM->SetHeight(heightA);
	pRestultDEM->SetWidth(widthA);
	if (!pRestultDEM->Allocate())
	{
		cout<<"Failed to allocate memory!"<<endl;
		return NULL;
	}

	float valueA, valueB;
	for (int row = 0; row < heightA; row++)
	{
		for (int col = 0; col < widthA; col++)
		{
			//保证要比较的点是在DEM中，而且是有效值
			if (demA.is_InGrid(row, col) && !demA.is_NoData(row, col) 
				&& demB.is_InGrid(row, col) && !demB.is_NoData(row, col))
			{
				valueA = demA.asFloat(row, col);
				valueB = demB.asFloat(row, col);

				float tmp = valueA - valueB;
				pRestultDEM->Set_Value(row, col, tmp);
			}
		}
	}
	return pRestultDEM;
}
