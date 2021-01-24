
def showMask(x):
    for i in range(64):
        print('*' if x>>i&1 else '.', end='')
        if i%8==7: print()

def showBoard(b, w):
    for i in range(64):
        if (b&w)>>i&1:
            raise
        elif b>>i&1:
            print('\u25cf',end='')
        elif w>>i&1:
            print('\u25cb',end='')
        else:
            print('\u00b7',end='')
        
        if i%8==7: print()

while True:
    try:
        s=input().replace(',',' ').split()
    except KeyboardInterrupt:
        break
    
    if len(s)==1:
        showMask(int(s[0], 16))
    else:
        showBoard(int(s[0], 16), int(s[1], 16))
