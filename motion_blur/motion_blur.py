import cv2
import numpy as np

def get_sequence(img, n):
    seq = []

    img_expanded = cv2.copyMakeBorder(img, img.shape[0] * 2, img.shape[0] * 2, 
        img.shape[1] * 2, img.shape[1] * 2, cv2.BORDER_CONSTANT)
    rows = img_expanded.shape[0]
    cols = img_expanded.shape[1]

    for i in range(0, n):
        M = np.float32([[1,0,8*i],[0,1,4*i]])
        dst = cv2.warpAffine(img_expanded,M,(cols,rows))
        seq.append(dst)

    return seq

img = cv2.imread('smallball.png',0)
img = cv2.cvtColor(img, cv2.COLOR_GRAY2RGB)

print(bg.shape)
print(img.shape)

seq = get_sequence(img, 8)

blur = np.copy(seq[0])

#for i in seq:
#    blur = cv2.addWeighted(blur, 0.5, i, 0.5, 0.2)

cv2.imwrite('res.png', img)
cv2.waitKey(0)