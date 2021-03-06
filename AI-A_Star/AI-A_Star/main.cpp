/* A Star Algorithm */
#include <iostream>
#include <iomanip>
#include <string.h>
#include <time.h>
#include <vector>
#include <queue>
#include <stack>
using namespace std;

#define PUZZLE_SIZE	3 				// 3x3 八数码

/* 八数码结构体 */
struct EightPuzzle
{
	int puzzle[PUZZLE_SIZE][PUZZLE_SIZE];
	int diff;
	int fatherLoc;
	int layer;
	long checkSum;
};

/* 定义priority_queue上从小到大的规则(同时也是A*的启发函数) */
bool operator > (const EightPuzzle &ep1,const EightPuzzle &ep2)
{
	return (ep1.diff+ep1.layer) > (ep2.diff+ep2.layer);
}

EightPuzzle g_epAim;				// 目标状态
EightPuzzle g_epStart;				// 开始状态

vector<EightPuzzle> Vec_EP;			// 结点历史数组 用于检查重复(Close表)
stack<EightPuzzle> Stack_EP;		// 结点栈 借以最终输出可行路径
priority_queue<EightPuzzle,vector<EightPuzzle>,greater<EightPuzzle>> Que_EP;	// 结点队列 用于搜索(Open表)	

/* 初始化问题前提 */
void InitAim();													// 初始化目标结点
void InputStartStatus();										// 输入起始结点

/* 计算属性值 */
int CalcPuzzleDiff(const EightPuzzle &epNow);					// 根据目标结点计算当前结点与其的差距
int CalcCheckSum(const EightPuzzle &ep);						// 根据结点八数码情况 生成校验码

/* 查询操作 */
int  GetLocInVec(const EightPuzzle &ep);						// 获取结点在结点历史数组里的位置 若不存在则返回-1
bool IsArriveAim(const EightPuzzle &ep);						// 判断是否到达目标结点

/* 基本操作 */
void CopyPuzzle(EightPuzzle &dest,const EightPuzzle &source);	// 拷贝结点八数码 其他属性不变
void PrintPuzzle(EightPuzzle &puzzleIn);						// 打印结点八数码情况
void PrintValidPath();											// 打印最终求出的有效路径

/* A*算法 */
int A_Start();													// A*算法主体
int CreateChildrenSet(EightPuzzle &epNow,EightPuzzle epArr[4]);	// 根据结点八数码情况 生成其子结点集合 epArr数组用于存放这些结点


/* 初始化目标结点 */
void InitAim()
{
	int puzzleAim[PUZZLE_SIZE][PUZZLE_SIZE] = {{1,2,3},{8,0,4},{7,6,5}};

	for(int i=0;i<PUZZLE_SIZE;i++)
		for(int j=0;j<PUZZLE_SIZE;j++)
			g_epAim.puzzle[i][j] = puzzleAim[i][j];

	g_epAim.diff = 0;
	g_epAim.layer = -1;
	g_epAim.fatherLoc = -1;
	g_epAim.checkSum = CalcCheckSum(g_epAim);

}

/* 输入起始结点 */
void InputStartStatus()
{
	// {1,3,0},{8,2,4},{7,6,5}	//	{2,8,0},{1,6,3},{7,5,4}	 //  {3,7,2},{8,1,5},{4,6,0}	// {8,3,2},{7,1,0},{4,6,5}
	//int puzzleStart[PUZZLE_SIZE][PUZZLE_SIZE] = {{8,3,2},{7,1,0},{4,6,5}};	
	cout<<"请输入9个数字，初始化八数码（用0表示空格）："<<endl;
	for(int i=0;i<PUZZLE_SIZE;i++)
		for(int j=0;j<PUZZLE_SIZE;j++)
			cin>>g_epStart.puzzle[i][j];
			//g_epStart.puzzle[i][j] = puzzleStart[i][j];
	g_epStart.diff = CalcPuzzleDiff(g_epStart);
	g_epStart.fatherLoc = -1;
	g_epStart.layer = 0;
	g_epStart.checkSum = CalcCheckSum(g_epStart);
}

/* 根据目标结点计算当前结点与其的差距 */
int CalcPuzzleDiff(const EightPuzzle &epNow)
{
	int diff = 0;
	for(int i=0;i<PUZZLE_SIZE;i++)
		for(int j=0;j<PUZZLE_SIZE;j++)
			if(epNow.puzzle[i][j] != g_epAim.puzzle[i][j]) diff++;
	return diff;
}

/* 根据结点八数码情况 生成校验码 */
int CalcCheckSum(const EightPuzzle &ep)
{
	int checkSum = 0;
	for(int i=0;i<PUZZLE_SIZE;i++)
	{
		for(int j=0;j<PUZZLE_SIZE;j++)
		{
			checkSum *= 10;
			checkSum += ep.puzzle[i][j];
		}
	}
	return checkSum;
}

/* 获取结点在结点历史数组里的位置 若不存在则返回-1 */
int GetLocInVec(const EightPuzzle &ep)
{
	int len = Vec_EP.size();
	for(int i=0;i<len;i++)
		if(ep.checkSum == Vec_EP[i].checkSum) return i;
	return -1;
}

/* 判断是否到达目标结点 */
bool IsArriveAim(const EightPuzzle &ep)
{
	if(ep.checkSum == g_epAim.checkSum) return true;
	else return false;
}

/* 拷贝结点八数码 其他属性不变 */
void CopyPuzzle(EightPuzzle &dest,const EightPuzzle &source)
{
	for(int i=0;i<PUZZLE_SIZE;i++)
		for(int j=0;j<PUZZLE_SIZE;j++)
			dest.puzzle[i][j] = source.puzzle[i][j];
}

/* 打印结点八数码情况 */
void PrintPuzzle(EightPuzzle &puzzleIn)
{
	for(int i=0;i<PUZZLE_SIZE;i++)
	{
		for(int j=0;j<PUZZLE_SIZE;j++)
		{
			if(puzzleIn.puzzle[i][j] != 0)
				cout<<setw(3)<<puzzleIn.puzzle[i][j];
			else
				cout<<setw(3)<<"";
		}	
		cout<<endl;
	}
	cout<<"Diff:"<<puzzleIn.diff<<endl;
	//cout<<"CheckSum:"<<puzzleIn.checkSum<<endl;
}
/* 打印最终求出的有效路径 */
void PrintValidPath()
{
	EightPuzzle ep = Vec_EP[Vec_EP.size() - 1];
	Stack_EP.push(ep);
	int step = 0;
	while(ep.fatherLoc != -1)
	{
		ep = Vec_EP[ep.fatherLoc];
		Stack_EP.push(ep);
	}
	while(!Stack_EP.empty())
	{
		cout<<"step:"<<step++<<endl;
		ep = Stack_EP.top();
		Stack_EP.pop();

		PrintPuzzle(ep);
		if(!Stack_EP.empty()) 
			cout<<endl;
	}
	cout<<endl;
}

/* 根据结点八数码情况 生成其子结点集合 epArr数组用于存放这些结点 */
int CreateChildrenSet(EightPuzzle &epNow,EightPuzzle epArr[4])
{
	/* 定位0的位置 */
	int locX,locY;
	bool flag = 1;
	for(locX = 0;locX < PUZZLE_SIZE;locX++)
	{
		for(locY = 0;locY < PUZZLE_SIZE;locY++)
			if(epNow.puzzle[locX][locY] == 0)
			{
				flag = 0;
				break;
			}
		if(!flag) break;
	}
	
	/* 尝试4个方向的移动 */
	int success = 0;
	int operation[4][2] = {{0,1},{0,-1},{1,0},{-1,0}}; 
	for(int i=0;i<4;i++)
	{
		/* 判断边界 若符合则加入数组中 */
		int newLocX = locX + operation[i][0];
		int newLocY = locY + operation[i][1];
		if( newLocX >= 0 && newLocX < PUZZLE_SIZE)
		{
			if(newLocY  >= 0 && newLocY < PUZZLE_SIZE)
			{
				CopyPuzzle(epArr[success],epNow);	// 拷贝原状态
				// 将0位置与相邻位置交换
				epArr[success].puzzle[locX][locY] = epArr[success].puzzle[newLocX][newLocY];
				epArr[success].puzzle[newLocX][newLocY] = 0;
				epArr[success].diff = CalcPuzzleDiff(epArr[success]);
				epArr[success].checkSum = CalcCheckSum(epArr[success]);
				epArr[success].layer = epNow.layer + 1;
				// 计数器加1
				success ++;
			}
		}
	}
	return success;
}

/* A*算法主体 */
int A_Start()
{
	EightPuzzle epNow;				// 创建当前结点
	EightPuzzle epChildArr[4];		// 用于保存结点的子树集

	Que_EP.push(g_epStart);			// 将起始结点存入Open表
	Vec_EP.push_back(g_epStart);	// 将起始结点存入Close表

	while(!Que_EP.empty())			// 搜索遍历 直到Open表为空
	{
		epNow = Que_EP.top();	Que_EP.pop();				// 取出Open表中优先级最高的结点

		int childNum = CreateChildrenSet(epNow,epChildArr);	// 产生子结点
		int fatherLoc = GetLocInVec(epNow);					// 当前结点的坐标即为孩子结点的父结点位置
		int repeatNodeLoc;

		for(int i=0;i<childNum;i++)
		{
			repeatNodeLoc = GetLocInVec(epChildArr[i]);		// 新结点进入close表中比对
			if(repeatNodeLoc != -1)							// 若与已存在的结点重复 则检查层数
			{
				// 若层数多的就更新覆盖 即优先级低的被覆盖
				if(Vec_EP[repeatNodeLoc].layer > epChildArr[i].layer)	
				{
					Vec_EP[repeatNodeLoc].layer = epChildArr[i].layer;
					Vec_EP[repeatNodeLoc].fatherLoc = fatherLoc;
				}
			}
			else	// 不与存在结点重复 则新增结点
			{
				epChildArr[i].fatherLoc = fatherLoc;
				Que_EP.push(epChildArr[i]);
				Vec_EP.push_back(epChildArr[i]);
				if(IsArriveAim(epChildArr[i]))
					return 1;
			}
		}
	}
	return 0;
}

int main()
{	
	InitAim();						// 设定目标状态
	InputStartStatus();				// 输入初始化状态

	clock_t start_time=clock();		// 程序计时
	
	/* 显示初始化状态和目标状态 */
	cout<<"初始化状态："<<endl;
	PrintPuzzle(g_epStart);
	cout<<endl;
	cout<<"目标状态："<<endl;
	PrintPuzzle(g_epAim);
	cout<<endl;

	/* 开始A*算法 若无解则返回0 */
	if(A_Start())
	{
		cout<<endl;
		PrintValidPath();
		cout<<"有解！最短路径如上。"<<endl<<endl;
	}else
		cout<<endl<<"该情况无解！"<<endl;

	clock_t end_time=clock();		// 程序计时

	cout<< "Running time is: "<<static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<endl;//输出运行时间

	return 0;
}