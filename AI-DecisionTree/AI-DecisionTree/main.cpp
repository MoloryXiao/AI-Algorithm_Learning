#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <fstream>
#include <iomanip>
using namespace std;

#define ELEMENT_NUM  4	// 属性个数

struct DataFormat
{
	double element[ELEMENT_NUM];
	int familyNum;
};
struct SpilitRecord
{
	int divideRule;
	double divideLine;
	double infoGain;
	double gainRatio;
};
struct SplitInfo
{
	int divideRule;				// 划分属性
	double divideLine;			// 划分界限
	double entropy;				// 当前熵值
};
typedef struct Node
{
	SplitInfo spif;				// 结点分裂的相关信息
	vector<int> subSet;			// 数据子集的标号集
	struct Node *lChild;
	struct Node *rChild;
	int familyNum;				// 当前类别
}BTreeNode,*BTree;
	
vector<DataFormat> g_DataGroup;	// 所有的数据集
vector<DataFormat> g_TestGroup;
vector<SpilitRecord> g_spilitRecords;

void BuildTree(BTree &node);
void InitRootNode(BTree &node);
void InitNewNode(BTreeNode &node);

bool cmp(const int &a,const int &b)
{
	return g_DataGroup[a].familyNum < g_DataGroup[b].familyNum;
}
bool cmp_ratio(const SpilitRecord &a,const SpilitRecord &b)
{
	return a.gainRatio > b.gainRatio;
}
bool cmp_gain(const SpilitRecord &a,const SpilitRecord &b)
{
	return a.infoGain > b.infoGain;
}
double log2(double p)
{
	return log(p)/log(2.0);
}

/* 计算结点熵值 赋值并返回该值 */
double CalcEntropy(BTreeNode &node)
{
	if(node.subSet.size() == 0) 
		return -1;

	double entropy = 0,pi = 0;
	double sampleSum = node.subSet.size();	// 样本总量
	sort(node.subSet.begin(),node.subSet.end(),cmp);	// 将数据按类别从小到大排列

	// 累加计算熵值
	int category = g_DataGroup[node.subSet[0]].familyNum;
	int sameCount = 0;
	for(int i=0;i<sampleSum;i++)
	{
		if(g_DataGroup[node.subSet[i]].familyNum == category)
		{
			sameCount++;
		}else
		{
			pi = sameCount*1.0/sampleSum;						// 某一类别的概率
			entropy += (-1)*pi*log2(pi);						// 累加熵值
			
			category = g_DataGroup[node.subSet[i]].familyNum;	// 更新比对值
			sameCount = 0;		// 计数器清零
			i--;				// 退格
		}
		if(i == sampleSum-1)
		{
			pi = sameCount*1.0/sampleSum;
			entropy += (-1)*pi*log2(pi);
		}
	}
	node.spif.entropy = entropy;
	return entropy;
}

// 结点根据选择的属性rule和选择的分裂点line 计算分裂信息熵和分裂信息
double CalcDivideEntropy(BTreeNode &node,int rule,double line,double &spilitE)
{
	BTreeNode lNode,rNode;
	int sampleSum = node.subSet.size();
	int lCount = 0,rCount = 0;
	double divideEntropy = 0;
	double piL = 0,piR = 0;

	for(int i=0;i<sampleSum;i++)
	{
		if(g_DataGroup[node.subSet[i]].element[rule] < line)
		{
			lNode.subSet.push_back(node.subSet[i]);
			lCount++;
		}
		else
		{
			rNode.subSet.push_back(node.subSet[i]);
			rCount++;
		}
	}
	piL = (lCount*1.0)/(sampleSum*1.0); 
	piR = (rCount*1.0)/(sampleSum*1.0);

	divideEntropy = piL * CalcEntropy(lNode) + piR * CalcEntropy(rNode);	// 划分信息熵
	spilitE = (-1)*piL*log2(piL) + (-1)*piR*log2(piR);						// 分裂信息

	return divideEntropy;
}

// 计算信息增益
double CalcGain(BTreeNode &node,int rule,double line,double &spilitE)
{
	return CalcEntropy(node) - CalcDivideEntropy(node,rule,line,spilitE);
}

// 根据结点获取所有可能分裂的情况
void GetAllSpilit(BTreeNode &node)
{
	g_spilitRecords.swap(vector<SpilitRecord>());			// 清空结果集

	int sampleSum = node.subSet.size();
	if(sampleSum < 2) return;

	double spilitPoint;		// 分裂点
	double spilitE;			// 分裂信息
	// 对每个属性中每两个数据元素取中点 作为分裂点
	for(int i=0;i<ELEMENT_NUM;i++)
	{
		for(int j=0;j<sampleSum-1;j++)
		{
			spilitPoint = g_DataGroup[node.subSet[j]].element[i] + g_DataGroup[node.subSet[j+1]].element[i];
			spilitPoint /= 2.0;				// 取两数据值的中点
				
			SpilitRecord sr;				// 新建一条分裂记录
			sr.divideLine = spilitPoint;	// 赋值分裂点
			sr.divideRule = i;				// 赋值划分属性
			sr.infoGain = CalcGain(node,i,spilitPoint,spilitE);	// 赋值信息增益 同时计算划分信息
			sr.gainRatio = sr.infoGain*1.0/spilitE;				// 赋值信息增益率

			g_spilitRecords.push_back(sr);
		}
	}
}

/* 获取最优的划分属性 */
int GetPerfectRule()
{
	// 对所有分裂策略取最高信息增益率的方案
	sort(g_spilitRecords.begin(),g_spilitRecords.end(),cmp_ratio);
	return g_spilitRecords[0].divideRule;	// 取该种方案的属性作为划分属性
}

/* 根据给定划分属性rule 到分裂策略集中找最佳分裂点 */
double GetPerfectLine(int rule)
{
	// 根据信息增益大小进行排序
	sort(g_spilitRecords.begin(),g_spilitRecords.end(),cmp_gain);
	// 找符合所给属性rule 并所得信息增益最大的的分裂点
	for(int i=0;i<g_spilitRecords.size();i++)
		if(g_spilitRecords[i].divideRule == rule)
			return g_spilitRecords[i].divideLine;
	return -1;
}

/* 对纯点进行验证 并返回类别 */
int GetCategory(BTreeNode &node)
{
	return g_DataGroup[node.subSet[0]].familyNum;
}

/* 根据当前结点的分裂信息 对该结点进行分裂 */
void Spilit(BTreeNode &node)
{
	int sampleSum = node.subSet.size();
	int spilitRule = node.spif.divideRule;
	double spilitLine = node.spif.divideLine;

	node.lChild = new BTreeNode; InitNewNode(*node.lChild);
	node.rChild = new BTreeNode; InitNewNode(*node.rChild);

	for(int i=0;i<sampleSum;i++)
	{
		if(g_DataGroup[node.subSet[i]].element[spilitRule] < spilitLine)
			node.lChild->subSet.push_back(node.subSet[i]);
		else
			node.rChild->subSet.push_back(node.subSet[i]);
	}
}

/* 初始化根结点数据 */
void InitRootNode(BTree &node)
{
	// 实例化根节点
	node = new BTreeNode;
	node->familyNum = -1;

	// 当前所有数据都处于根节点
	int dataSum = g_DataGroup.size();
	for(int i=0;i<dataSum;i++)
		node->subSet.push_back(i);
}
/* 初始化新结点 处理包含的指针等 */
void InitNewNode(BTreeNode &node)
{
	node.lChild = NULL; node.rChild = NULL;
	node.familyNum = -1;
	node.spif.divideLine = -1;
	node.spif.divideRule = -1;
	node.spif.entropy = -1;
}

void BuildTree(BTree &node)
{
	int perfectRule;		// 最佳划分属性
	double perfectLine;		// 最佳分裂点
	if(CalcEntropy(*node) == 0)			// 该结点为纯点 无需建立子树
	{
		node->lChild = NULL;	node->rChild = NULL;
		node->spif.divideRule = -1;	node->spif.divideLine = -1;
		node->familyNum = GetCategory(*node);
		return;
	}else		// 尚有杂质 继续分裂
	{
		// 对当前结点生成所有分裂方案
		GetAllSpilit(*node);

		// 选择信息增益率最大的作为分裂属性
		perfectRule = GetPerfectRule();
		node->spif.divideRule = perfectRule;

		// 选定划分属性后 选择信息增益最大的作为分裂界线
		perfectLine = GetPerfectLine(node->spif.divideRule);
		node->spif.divideLine = perfectLine;

		// 分裂
		Spilit(*node);

		// 递归生成子树
		BuildTree(node->lChild);
		BuildTree(node->rChild);
	}
}
void debug()
{
	DataFormat t1,t2,t3,t4,t5,t6,t7,t8,t9;
	t1.element[0] = 5.0; t1.element[1] = 3.0; t1.element[2] = 1.6; t1.element[3] = 0.2; t1.familyNum = 1; g_DataGroup.push_back(t1);
	t2.element[0] = 5.0; t2.element[1] = 3.4; t2.element[2] = 1.6; t2.element[3] = 0.4; t2.familyNum = 1; g_DataGroup.push_back(t2);
	t3.element[0] = 5.2; t3.element[1] = 3.5; t3.element[2] = 1.5; t3.element[3] = 0.2; t3.familyNum = 1; g_DataGroup.push_back(t3);
	t4.element[0] = 6.7; t4.element[1] = 3.0; t4.element[2] = 4.4; t4.element[3] = 1.4; t4.familyNum = 2; g_DataGroup.push_back(t4);
	t5.element[0] = 6.8; t5.element[1] = 2.8; t5.element[2] = 4.8; t5.element[3] = 1.4; t5.familyNum = 2; g_DataGroup.push_back(t5);
	t6.element[0] = 6.7; t6.element[1] = 3.0; t6.element[2] = 5.0; t6.element[3] = 1.7; t6.familyNum = 2; g_DataGroup.push_back(t6);
	t7.element[0] = 6.5; t7.element[1] = 3.0; t7.element[2] = 5.2; t4.element[3] = 2.0; t7.familyNum = 3; g_DataGroup.push_back(t7);
	t8.element[0] = 6.2; t8.element[1] = 3.4; t8.element[2] = 5.4; t8.element[3] = 2.3; t8.familyNum = 3; g_DataGroup.push_back(t8);
	t9.element[0] = 5.9; t9.element[1] = 3.0; t9.element[2] = 5.1; t9.element[3] = 1.8; t9.familyNum = 3; g_DataGroup.push_back(t9);
}
void TrainDataIn()
{
	ifstream ifs("traindata.txt");
	if(ifs == NULL) return;

	DataFormat td;
	while(ifs>>td.element[0]>>td.element[1]>>td.element[2]>>td.element[3]>>td.familyNum)
	{
		/*
		DataFormat td;
		for(int i=0;i<ELEMENT_NUM;i++)
			ifs>>td.element[i];
		ifs>>td.familyNum;
		*/
		g_DataGroup.push_back(td);
	}
	ifs.close();
}
void TestDataIn()
{
	ifstream ifs("testdata.txt");
	if(ifs == NULL) return;

	DataFormat td;
	while(ifs>>td.element[0]>>td.element[1]>>td.element[2]>>td.element[3]>>td.familyNum)
	{
		/*
		DataFormat td;
		for(int i=0;i<ELEMENT_NUM;i++)
			ifs>>td.element[i];
		ifs>>td.familyNum;
		*/
		g_TestGroup.push_back(td);
	}
	ifs.close();
}
void TreePrint(BTree T,int level)
{  
    if(!T)							//如果指针为空，返回上一层  
    {  
        return;  
    }  
    TreePrint(T->rChild,level+1);	//打印右子树，并将层次加1  
    for (int i=0;i<level;i++)		//按照递归的层次打印空格  
    {  
        printf("   ");  
    }
	if(T->familyNum != -1)
		cout<<T->familyNum<<endl;	//输出根结点
	else
		cout<<(char)(T->spif.divideRule + 'A')<<endl;
    TreePrint(T->lChild,level+1);	//打印左子树，并将层次加1  
}
void SearchResult(BTree &node,const DataFormat &data,int &result)
{
	if(node->spif.entropy == 0)
	{
		result = node->familyNum;
		return;
	}

	int spilitRule = node->spif.divideRule;
	double spilitPoint = node->spif.divideLine;
	if(data.element[spilitRule] < spilitPoint) SearchResult(node->lChild,data,result);
	else SearchResult(node->rChild,data,result);
}
void VerifyTestData(BTree &node)
{
	cout<<setw(5)<<"A"<<setw(5)<<"B"<<setw(5)<<"C"<<setw(5)<<"D"
		<<setw(8)<<"U GIVE"<<setw(6)<<"I GET"<<setw(5)<<" "<<endl;
	
	int testSum = g_TestGroup.size();
	int correct = 0;
	int result;
	for(int i=0;i<testSum;i++)
	{
		SearchResult(node,g_TestGroup[i],result);
		for(int j=0;j<ELEMENT_NUM;j++)
			cout<<setw(5)<<g_TestGroup[i].element[j];
		cout<<setw(8)<<g_TestGroup[i].familyNum<<setw(6)<<result;
		if(result == g_TestGroup[i].familyNum)
		{
			correct++;
			cout<<setw(5)<<"√"<<endl;
		}
		else 
			cout<<setw(5)<<"×"<<endl;
	}
	cout<<"正确率："<<(correct*1.0)/(testSum*1.0)<<endl;
}
int main()
{
	//debug();
	TrainDataIn();
	TestDataIn();
	BTree decisionTree;
	InitRootNode(decisionTree);
	BuildTree(decisionTree);
	TreePrint(decisionTree,0);
	VerifyTestData(decisionTree);
	return 0;
}