import cv2
import numpy as np
from matplotlib import pyplot as plt

ein = cv2.imread('ein250.png', 0)
mon = cv2.imread('mon250.png', 0)

high = cv2.Sobel(ein, -1, 1, 1, scale=1, delta=128)

low = cv2.GaussianBlur(mon, (7,7), 10)

print(high.shape)
print(low.shape)

res = cv2.addWeighted(high, 0.7, low, 0.3, 0)

cv2.imshow("aa", res)
cv2.waitKey()