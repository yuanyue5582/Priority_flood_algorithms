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
#include <stack>
#include <vector>
#include <chrono>
using namespace std;

typedef std::vector<Node> NodeVector;
typedef std::priority_queue<Node, NodeVector, Node::Greater> PriorityQueue;
size_t priorityNodes = 0;
// 添加安全访问宏
#define SAFE_ACCESS(row, col, height, width) \
    ((row) >= 0 && (row) < static_cast<int>(height) && \
     (col) >= 0 && (col) < static_cast<int>(width))

void InitPriorityQue(CDEM& dem, Flag& flag, PriorityQueue& priorityQueue)
{
	const size_t width = dem.Get_NX();
	const size_t height = dem.Get_NY();
	Node tmpNode;
	int iRow, iCol, row, col;

	queue<Node> depressionQue;

	// push border cells into the PQ
	for (row = 0; row < static_cast<int>(height); row++)
	{
		for (col = 0; col < static_cast<int>(width); col++)
		{
			if (flag.IsProcessedDirect(row, col)) continue;

			if (dem.is_NoData(row, col)) {
				flag.SetFlag(row, col);
				for (int i = 0; i < 8; i++)
				{
					iRow = Get_rowTo(i, row);
					iCol = Get_colTo(i, col);
					// 严格边界检查
					if (!SAFE_ACCESS(iRow, iCol, height, width) ||
						flag.IsProcessed(iRow, iCol) ||
						dem.is_NoData(iRow, iCol)) {
						continue;
					}

					tmpNode.row = iRow;
					tmpNode.col = iCol;
					tmpNode.spill = dem.asFloat(iRow, iCol);
					priorityQueue.push(tmpNode);
					priorityNodes++;

					flag.SetFlag(iRow, iCol);
				}
			}
			else
			{
				if (row == 0 || row == static_cast<int>(height) - 1 || col == 0 || col == static_cast<int>(width) - 1) {
					//on the DEM border
					tmpNode.row = row;
					tmpNode.col = col;
					tmpNode.spill = dem.asFloat(row, col);
					priorityQueue.push(tmpNode);
					priorityNodes++;
					flag.SetFlag(row, col);
				}
			}
		}
	}
}
void ProcessTraceQue(CDEM& dem, Flag& flag, queue<Node>& traceQueue, PriorityQueue& priorityQueue)
{
	bool HaveSpillPathOrLowerSpillOutlet;
	int i, iRow, iCol;
	int k, kRow, kCol;
	int noderow, nodecol;
	Node N, node;
	queue<Node> potentialQueue;
	int indexThreshold = 2;  //index threshold, default to 2
	while (!traceQueue.empty())
	{
		node = traceQueue.front();
		traceQueue.pop();
		noderow = node.row;
		nodecol = node.col;
		//initialize all masks as false	
		bool Mask[5][5] = { {false},{false},{false},{false},{false} };
		for (i = 0; i < 8; i++) {
			iRow = Get_rowTo(i, noderow);
			iCol = Get_colTo(i, nodecol);
			if (flag.IsProcessedDirect(iRow, iCol)) continue;
			if (dem.asFloat(iRow, iCol) > node.spill) {
				N.col = iCol;
				N.row = iRow;
				N.spill = dem.asFloat(iRow, iCol);
				traceQueue.push(N);
				flag.SetFlag(iRow, iCol);
			}
			else {
				//如果单元格 i 是一个洼地单元格，判断它是否有一个溢流路径，或者是否有一个比节点更低的溢流出口。	
				//所有已处理的邻居都没有比当前单元格更低的高程
				HaveSpillPathOrLowerSpillOutlet = false; //whether cell i has a spill path or a lower spill outlet than node if i is a depression cell
				for (k = 0; k < 8; k++) {
					kRow = Get_rowTo(k, iRow);
					kCol = Get_colTo(k, iCol);
					if ((Mask[kRow - noderow + 2][kCol - nodecol + 2]) ||
						(flag.IsProcessedDirect(kRow, kCol) && dem.asFloat(kRow, kCol) < node.spill)
						)
					{
						Mask[iRow - noderow + 2][iCol - nodecol + 2] = true;
						HaveSpillPathOrLowerSpillOutlet = true;
						break;
					}
				}
				if (!HaveSpillPathOrLowerSpillOutlet) {
					if (i < indexThreshold) potentialQueue.push(node);
					else {
						priorityQueue.push(node);
						priorityNodes++;
					}

					break; // make sure node is not pushed twice into PQ
				}
			}
		}//end of for loop
	}

	while (!potentialQueue.empty())
	{
		node = potentialQueue.front();
		potentialQueue.pop();
		noderow = node.row;
		nodecol = node.col;

		//first case
		for (i = 0; i < 8; i++)
		{
			iRow = Get_rowTo(i, noderow);
			iCol = Get_colTo(i, nodecol);
			if (flag.IsProcessedDirect(iRow, iCol)) continue;
			else {
				priorityQueue.push(node);
				priorityNodes++;
				break;
			}
		}
	}
}

void ProcessPit(CDEM& dem, Flag& flag, queue<Node>& depressionQue,
	queue<Node>& traceQueue, PriorityQueue& priorityQueue)
{
	int iRow, iCol, i;
	float iSpill;
	Node N;
	Node node;
	const size_t width = dem.Get_NX();
	const size_t height = dem.Get_NY();
	while (!depressionQue.empty())
	{
		node = depressionQue.front();
		depressionQue.pop();
		for (i = 0; i < 8; i++)
		{
			iRow = Get_rowTo(i, node.row);
			iCol = Get_colTo(i, node.col);
			if (flag.IsProcessedDirect(iRow, iCol)) continue;
			iSpill = dem.asFloat(iRow, iCol);
			if (iSpill > node.spill)
			{ //slope cell
				N.row = iRow;
				N.col = iCol;
				N.spill = iSpill;
				flag.SetFlag(iRow, iCol);
				traceQueue.push(N);
				continue;
			}
			//depression cell
			flag.SetFlag(iRow, iCol);
			dem.Set_Value(iRow, iCol, node.spill);
			N.row = iRow;
			N.col = iCol;
			N.spill = node.spill;
			depressionQue.push(N);
		}
	}
}
void fillDEM_Wei(const char* inputFile, const char* outputFilledPath)
{
	queue<Node> traceQueue;
	queue<Node> depressionQue;
	//read float-type DEM
	CDEM dem;
	double geoTransformArgs[6];
	std::cout << "Reading input tiff file..." << endl;
	if (!readTIFF(inputFile, GDALDataType::GDT_Float32, dem, geoTransformArgs)) {
		printf("Error occurred while reading GeoTIFF file!\n");
		return;
	}
	std::cout << "Finish reading file" << endl;

	const size_t width = static_cast<size_t>(dem.Get_NX());
	const size_t height = static_cast<size_t>(dem.Get_NY());
	std::cout << "Using our proposed variant to fill DEM" << endl;
	auto timeStart = std::chrono::high_resolution_clock::now();
	Flag flag;
	if (!flag.Init(width, height)) {
		printf("Failed to allocate memory!\n");
		return;
	}
	PriorityQueue priorityQueue;
	int iRow, iCol, row, col;
	float iSpill, spill;

	int numberofall = 0;
	int numberofright = 0;

	InitPriorityQue(dem, flag, priorityQueue);
	while (!priorityQueue.empty())
	{
		Node tmpNode = priorityQueue.top();
		priorityQueue.pop();
		row = tmpNode.row;
		col = tmpNode.col;
		spill = tmpNode.spill;

		for (int i = 0; i < 8; i++)
		{
			iRow = Get_rowTo(i, row);
			iCol = Get_colTo(i, col);

			if (flag.IsProcessed(iRow, iCol)) continue;
			iSpill = dem.asFloat(iRow, iCol);
			if (iSpill <= spill) {
				//depression cell
				dem.Set_Value(iRow, iCol, spill);
				flag.SetFlag(iRow, iCol);
				tmpNode.row = iRow;
				tmpNode.col = iCol;
				tmpNode.spill = spill;
				depressionQue.push(tmpNode);
				ProcessPit(dem, flag, depressionQue, traceQueue, priorityQueue);
			}
			else
			{
				//slope cell
				flag.SetFlag(iRow, iCol);
				tmpNode.row = iRow;
				tmpNode.col = iCol;
				tmpNode.spill = iSpill;
				traceQueue.push(tmpNode);
			}
			ProcessTraceQue(dem, flag, traceQueue, priorityQueue);
		}
	}
	// 记录结束时间  
	auto timeEnd = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> consumeTime = timeEnd - timeStart;
	cout << "Time used:" << consumeTime.count() << " seconds" << endl;
	std::cout << "\n===== 优先队列处理的节点数 =====\n" << priorityNodes << "\n";

	double min, max, mean, stdDev;
	calculateStatistics(dem, &min, &max, &mean, &stdDev);
	CreateGeoTIFF(outputFilledPath, dem.Get_NY(), dem.Get_NX(),
		(void*)dem.getDEMdata(), GDALDataType::GDT_Float32, geoTransformArgs,
		&min, &max, &mean, &stdDev, -9999);
	return;
}