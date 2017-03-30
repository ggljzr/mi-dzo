import cv2
import numpy as np

bg = None

def get_sequence(img, n):
    seq = []

    rows,cols = img.shape

    for i in range(0, n):
        M = np.float32([[1,0,8*i],[0,1,4*i]])
        dst = cv2.warpAffine(img,M,(cols,rows))
        seq.append(dst)

    return seq

img = cv2.imread('smallball.png',0)
img = cv2.copyMakeBorder(img, 100, 100, 100, 100, cv2.BORDER_CONSTANT)
print(img.shape)

seq = get_sequence(img, 8)

res = np.copy(img)

for i in seq:
    res = cv2.addWeighted(res, 0.5, i, 0.5, 0.2)



cv2.imwrite('res.png', res)
cv2.waitKey(0)
cv2.destroyAllWindows()