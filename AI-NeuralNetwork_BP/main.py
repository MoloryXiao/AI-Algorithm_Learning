
import numpy as np
from NeuralNetwork import NeuralNetwork

# 打开训练数据及测试数据文件
trainDataFile = open('Iris-data.txt')
testDataFile = open('Iris-test.txt')

# 读取文件内容
trainData = trainDataFile.readlines()
testData = testDataFile.readlines()

# 记录数据行数
trainRecordsNum = len(trainData)
testRecordsNum = len(testData)

# 创建数据矩阵
trainMatrixX = np.ones((trainRecordsNum,4))
testMatrix = np.ones((testRecordsNum,5))
 
trainDataLabel = []
#matrixY = []
currentLine = 0

for line in trainData:
    # 处理每行数据
    pData1 = line.strip('\n').split(' ')
    pData2 = [float(x) for x in pData1]
    
    trainMatrixX[currentLine] = pData2[0:-1]
    trainDataLabel.append(pData2[-1])

    currentLine += 1
print(trainMatrixX)
#print(matrixY)
print(trainDataLabel)

# 标签去重复
trainDataLabel = list(set(trainDataLabel))
trainMatrixY = np.zeros( ( trainRecordsNum,len(trainDataLabel) ) )

currentLine = 0
for line in trainData:
    # 处理每行数据
    pData1 = line.strip('\n').split(' ')
    pData2 = [float(x) for x in pData1]
    
    for i in range(len(trainDataLabel)):
        if pData2[-1] == trainDataLabel[i]:
            trainMatrixY[currentLine][i] = 1.0
            break
    currentLine += 1
    

nn = NeuralNetwork([4, 10, len(trainDataLabel)], 'logistic')
nn.fit(trainMatrixX, trainMatrixY)

currentLine = 0
for line in testData:
    pData1 = line.strip('\n').split(' ')  
    pData2 = [float(x) for x in pData1]
    testMatrix[currentLine] = pData2
    currentLine += 1
print(testMatrix)

correctCount = 0
for i in testMatrix:
    outputMat = nn.predict(i[0:-1])
    print(i,outputMat)
    outputList = outputMat.tolist()
    labelLoc = outputList.index(max(outputList))
    if trainDataLabel[labelLoc] == i[-1]:
        correctCount += 1
    print("predict Label:"+str(trainDataLabel[labelLoc]))
print("correct:"+str(correctCount))
print("correct rate:"+str(correctCount*1.0/testRecordsNum*100)+"%")
