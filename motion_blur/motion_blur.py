import cv2
import numpy as np
import sys

def get_sequence(img, n, x = 8, y = 4):
    seq = []

    img_expanded = cv2.copyMakeBorder(img, 0, n*y, 0, n*x, cv2.BORDER_CONSTANT)
    rows = img_expanded.shape[0]
    cols = img_expanded.shape[1]

    for i in range(0, n):
        M = np.float32([[1,0,x*i],[0,1,y*i]])
        dst = cv2.warpAffine(img_expanded,M,(cols,rows))
        seq.append(dst)

    return seq

im_name = 'smallball.png'

if len(sys.argv) > 1:
    im_name = sys.argv[1]

img = cv2.imread(im_name)

seq = get_sequence(img, 20)

blur = np.copy(seq[0])

for i in seq:
    blur = cv2.addWeighted(blur, 0.5, i, 0.5, 0.2)

cv2.imwrite('res.png', img)
cv2.waitKey(0)