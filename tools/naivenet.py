import torch
import torch.utils.data
from torch.nn import Module
import torch.nn.functional as F
from torch.utils.data import DataLoader, Dataset
import numpy as np
import numpy.ctypeslib as npct
from ctypes import c_int, c_float, c_uint64, CDLL, WinDLL
import os

Ninput=54
array_1d_i64 = npct.ndpointer(dtype=np.int64, ndim=1, flags='CONTIGUOUS')

# load the library, using numpy mechanisms
libbf = npct.load_library("libboardfeature.so", '.')
libbf.init()

# setup the return typs and argument types
libbf.get_feature.restype = c_int
libbf.get_feature.argtypes = [c_uint64, c_uint64, c_int, array_1d_i64]


class BoardData(Dataset):
    def __init__(self, filename):
        self.len=0
        self.board=[]
        self.value=[]

        vsum = 0
        with open(filename,'r') as f:
            for line in f.readlines():
                b1, b2, v=line.split()
                self.board.append((int(b1, 16), int(b2, 16)))
                self.value.append(float(v))
                self.len+=1
        vsum/=self.len
        print(f"{self.len} boards, average v={vsum}")
    
    def __len__(self):
        return self.len
    
    def __getitem__(self, index):
        farray = np.ndarray(Ninput, dtype=np.int64)
        n = libbf.get_feature(self.board[index][0], self.board[index][1], 1, farray)
        assert(n==Ninput)
        return farray[:6], farray[6:Ninput], self.value[index], 0

class FC(Module):
    def __init__(self):
        super(FC, self).__init__()
        boardfeatureslen = {
            'e1': 10,
            'c52': 10,
            'c33': 9,
            'e2': 8,
            'e3': 8,
            'e4': 8,
            'k8': 8,
            'k7': 7,
            'k6': 6,
            'k5': 5,
            'k4': 4,
            'ccor': 4,
            'cx22': 4
        }
        
        order="e2 "*4 + "e3 "*4 + "e4 "*4 + "k8 "*2 + "k7 "*4 + "k6 "*4 + "k5 "*4 + "k4 "*4
        order=order+"ccor cx22 e1 c33 c52 c33 c52 e1 c52 e1 c52 e1 c33 c52 c33 c52 c52 c52"
        order=order.split(" ")
        self.order = order

        self.boardfeature={}
        for k, v in boardfeatureslen.items():
            if v>8:
                self.boardfeature[k]=torch.nn.Parameter(torch.zeros(4,3**v, requires_grad=True))
                self.register_parameter(k, self.boardfeature[k])
            else:
                self.boardfeature[k]=torch.nn.Parameter(torch.zeros(4,4**v, requires_grad=True))
                self.register_parameter(k, self.boardfeature[k])

        self.nchannel = 1
        hidden = 64
        self.layer=torch.nn.Sequential(torch.nn.Linear(48*self.nchannel+6, hidden), torch.nn.ReLU(),
                                       torch.nn.Linear(hidden, hidden), torch.nn.ReLU()
                                        )
        self.out=torch.nn.Linear(hidden, 1)
        self.linear=torch.nn.Linear(48*self.nchannel+6, 1)
        print("nchannel:", self.nchannel)
    
    def forward_xboard(self, xboard):
        vboard = []
        for i,o in enumerate(self.order):
            for j in range(self.nchannel):
                vboard.append(self.boardfeature[o][j,xboard[:,i]].reshape(-1,1))

        return vboard

    def forward(self, xvalue, xboard):
        x=torch.cat([xvalue] + self.forward_xboard(xboard), dim=1)
        #return self.linear(x).reshape(-1)

        hidden=self.layer(x)
        out=self.out(hidden)
        return out.reshape(-1)


def testnaive(testdata):
    sum_loss=0
    accurancy=0
    testloader=DataLoader(testdata, batch_size=64, shuffle=False)
    
    for xvalue, xboard, value, dv in testloader:
        loss = torch.sum((value-dv)**2)
        sum_loss+=loss.data
        batch_acc=torch.sum(torch.sign(value)==torch.sign(dv))
        accurancy+=batch_acc
    print('test loss:%.03f' % (sum_loss / len(testdata)), end='')
    print('  correct:%.03f%%' % (100 * accurancy / len(testdata)))


def test(net, testdata):
    sum_mse=0
    sum_mae=0
    accurancy=0
    testloader=DataLoader(testdata, batch_size=64, shuffle=False)
    
    for xvalue, xboard, value, dv in testloader:
        v=net(xvalue, xboard)
        sum_mse+=torch.sum((value-v)**2)
        sum_mae+=torch.sum((value-v).abs()).data
        batch_acc=torch.sum(torch.sign(value)==torch.sign(v))
        accurancy+=batch_acc
    print('test mse:%.03f mae:%.03f' % (sum_mse / len(testdata), sum_mae/ len(testdata)), end='')
    print('  correct:%.03f%%' % (100 * accurancy / len(testdata)))

def train(net: Module, data):
    batch_size = 256
    testdata, traindata = torch.utils.data.random_split(data, [len(data)//20, len(data)-len(data)//20])
    loader=DataLoader(traindata, batch_size=batch_size, shuffle=True)

    optimizer = torch.optim.Adam(net.parameters(), lr=3e-3)
    epochs=10

    #testnaive(testdata)
    test(net, testdata)

    loss_func = 'mse'
    print("loss_func:", loss_func)
    
    for epoch in range(epochs):
        #print(net.forward_xboard(torch.tensor([traindata[0][1]])))
        sum_mse=0
        sum_mae=0
        accurancy=0
        net.train()
        for xvalue, xboard, value, dv in loader:
            optimizer.zero_grad()
            v=net(xvalue, xboard)
            mse = (value-v)**2
            mae = (value-v).abs()

            if loss_func=='mse':
                loss = torch.sum(mse)
            else:
                loss = torch.sum(mae)
            loss.backward()
            optimizer.step()
            
            sum_mse+= torch.sum(mse).data
            sum_mae+= torch.sum(mae).data
            batch_acc=torch.sum(torch.sign(value)==torch.sign(v))
            accurancy+=batch_acc
        
        print('[%d,%d] mse:%.03f mae:%0.03f' % (epoch + 1, epochs, sum_mse / len(traindata), sum_mae / len(traindata)), end='')
        print('  correct:%.03f%%' % (100 * accurancy / len(traindata)))
        #print(net.boardfeature['cx22'])

        net.eval()
        test(net, testdata)

    #torch.save(net, 'data/FCnet.pth')

if __name__=='__main__':
    train(FC(), BoardData('data/rawdata3/data8_10.txt'))
