#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <fstream>
#include <iomanip>
using namespace std;

#define ELEMENT_NUM  4	// ���Ը���

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
	int divideRule;				// ��������
	double divideLine;			// ���ֽ���
	double entropy;				// ��ǰ��ֵ
};
typedef struct Node
{
	SplitInfo spif;				// �����ѵ������Ϣ
	vector<int> subSet;			// �����Ӽ��ı�ż�
	struct Node *lChild;
	struct Node *rChild;
	int familyNum;				// ��ǰ���
}BTreeNode,*BTree;
	
vector<DataFormat> g_DataGroup;	// ���е����ݼ�
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

/* ��������ֵ ��ֵ�����ظ�ֵ */
double CalcEntropy(BTreeNode &node)
{
	if(node.subSet.size() == 0) 
		return -1;

	double entropy = 0,pi = 0;
	double sampleSum = node.subSet.size();	// ��������
	sort(node.subSet.begin(),node.subSet.end(),cmp);	// �����ݰ�����С��������

	// �ۼӼ�����ֵ
	int category = g_DataGroup[node.subSet[0]].familyNum;
	int sameCount = 0;
	for(int i=0;i<sampleSum;i++)
	{
		if(g_DataGroup[node.subSet[i]].familyNum == category)
		{
			sameCount++;
		}else
		{
			pi = sameCount*1.0/sampleSum;						// ĳһ���ĸ���
			entropy += (-1)*pi*log2(pi);						// �ۼ���ֵ
			
			category = g_DataGroup[node.subSet[i]].familyNum;	// ���±ȶ�ֵ
			sameCount = 0;		// ����������
			i--;				// �˸�
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

// ������ѡ�������rule��ѡ��ķ��ѵ�line ���������Ϣ�غͷ�����Ϣ
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

	divideEntropy = piL * CalcEntropy(lNode) + piR * CalcEntropy(rNode);	// ������Ϣ��
	spilitE = (-1)*piL*log2(piL) + (-1)*piR*log2(piR);						// ������Ϣ

	return divideEntropy;
}

// ������Ϣ����
double CalcGain(BTreeNode &node,int rule,double line,double &spilitE)
{
	return CalcEntropy(node) - CalcDivideEntropy(node,rule,line,spilitE);
}

// ���ݽ���ȡ���п��ܷ��ѵ����
void GetAllSpilit(BTreeNode &node)
{
	g_spilitRecords.swap(vector<SpilitRecord>());			// ��ս����

	int sampleSum = node.subSet.size();
	if(sampleSum < 2) return;

	double spilitPoint;		// ���ѵ�
	double spilitE;			// ������Ϣ
	// ��ÿ��������ÿ��������Ԫ��ȡ�е� ��Ϊ���ѵ�
	for(int i=0;i<ELEMENT_NUM;i++)
	{
		for(int j=0;j<sampleSum-1;j++)
		{
			spilitPoint = g_DataGroup[node.subSet[j]].element[i] + g_DataGroup[node.subSet[j+1]].element[i];
			spilitPoint /= 2.0;				// ȡ������ֵ���е�
				
			SpilitRecord sr;				// �½�һ�����Ѽ�¼
			sr.divideLine = spilitPoint;	// ��ֵ���ѵ�
			sr.divideRule = i;				// ��ֵ��������
			sr.infoGain = CalcGain(node,i,spilitPoint,spilitE);	// ��ֵ��Ϣ���� ͬʱ���㻮����Ϣ
			sr.gainRatio = sr.infoGain*1.0/spilitE;				// ��ֵ��Ϣ������

			g_spilitRecords.push_back(sr);
		}
	}
}

/* ��ȡ���ŵĻ������� */
int GetPerfectRule()
{
	// �����з��Ѳ���ȡ�����Ϣ�����ʵķ���
	sort(g_spilitRecords.begin(),g_spilitRecords.end(),cmp_ratio);
	return g_spilitRecords[0].divideRule;	// ȡ���ַ�����������Ϊ��������
}

/* ���ݸ�����������rule �����Ѳ��Լ�������ѷ��ѵ� */
double GetPerfectLine(int rule)
{
	// ������Ϣ�����С��������
	sort(g_spilitRecords.begin(),g_spilitRecords.end(),cmp_gain);
	// �ҷ�����������rule ��������Ϣ�������ĵķ��ѵ�
	for(int i=0;i<g_spilitRecords.size();i++)
		if(g_spilitRecords[i].divideRule == rule)
			return g_spilitRecords[i].divideLine;
	return -1;
}

/* �Դ��������֤ ��������� */
int GetCategory(BTreeNode &node)
{
	return g_DataGroup[node.subSet[0]].familyNum;
}

/* ���ݵ�ǰ���ķ�����Ϣ �Ըý����з��� */
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

/* ��ʼ����������� */
void InitRootNode(BTree &node)
{
	// ʵ�������ڵ�
	node = new BTreeNode;
	node->familyNum = -1;

	// ��ǰ�������ݶ����ڸ��ڵ�
	int dataSum = g_DataGroup.size();
	for(int i=0;i<dataSum;i++)
		node->subSet.push_back(i);
}
/* ��ʼ���½�� ���������ָ��� */
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
	int perfectRule;		// ��ѻ�������
	double perfectLine;		// ��ѷ��ѵ�
	if(CalcEntropy(*node) == 0)			// �ý��Ϊ���� ���轨������
	{
		node->lChild = NULL;	node->rChild = NULL;
		node->spif.divideRule = -1;	node->spif.divideLine = -1;
		node->familyNum = GetCategory(*node);
		return;
	}else		// �������� ��������
	{
		// �Ե�ǰ����������з��ѷ���
		GetAllSpilit(*node);

		// ѡ����Ϣ������������Ϊ��������
		perfectRule = GetPerfectRule();
		node->spif.divideRule = perfectRule;

		// ѡ���������Ժ� ѡ����Ϣ����������Ϊ���ѽ���
		perfectLine = GetPerfectLine(node->spif.divideRule);
		node->spif.divideLine = perfectLine;

		// ����
		Spilit(*node);

		// �ݹ���������
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
    if(!T)							//���ָ��Ϊ�գ�������һ��  
    {  
        return;  
    }  
    TreePrint(T->rChild,level+1);	//��ӡ��������������μ�1  
    for (int i=0;i<level;i++)		//���յݹ�Ĳ�δ�ӡ�ո�  
    {  
        printf("   ");  
    }
	if(T->familyNum != -1)
		cout<<T->familyNum<<endl;	//��������
	else
		cout<<(char)(T->spif.divideRule + 'A')<<endl;
    TreePrint(T->lChild,level+1);	//��ӡ��������������μ�1  
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
			cout<<setw(5)<<"��"<<endl;
		}
		else 
			cout<<setw(5)<<"��"<<endl;
	}
	cout<<"��ȷ�ʣ�"<<(correct*1.0)/(testSum*1.0)<<endl;
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