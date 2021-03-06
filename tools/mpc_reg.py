# an experimental linear regression for probcut
import numpy as np

full_data=np.loadtxt("data/pc_ndata.txt", delimiter=',')

# w, b, sigma
result=np.ndarray([64,15,3])
result[:,:]=[1,0,100]

for cnt in range(6,60):
    for dep in range(3,15):
        data=full_data[full_data[:,0]==dep]
        data=data[data[:,1]==cnt]
        print(f"dep:{dep}, cnt:{cnt}, len:{len(data)}")
        if (len(data)<=5):
            if len(data)>=1:
                print("lack of data:")
                print(data)
            continue
        x=data[:,2]
        y=data[:,3]
        w,b=np.polyfit(x,y,1)
        sigma=np.std(w*x+b-y, ddof=1) + 10/len(data)
        print(f"w:{w}, b:{b}, sigma:{sigma}")
        result[cnt,dep] = [w, b, sigma]

np.savetxt("data/pc_coeff.txt", result.reshape([-1,3]), fmt="%.4f")
