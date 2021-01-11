import math

ANGLE_IN_DEGREE = 1

positions = [i*30 for i in range(8)]
angles    = []

print "positions: ", positions

for pos in positions:
    
    a = 100 - pos
    b = math.sqrt(25**2 + 100**2 + a**2)
    c = math.sqrt(25**2 + 100**2)
    
    print "a, b, c = ", a, b, c
    
    angle = math.acos((a**2 + b**2 - c**2)/(2*a*b))
    if ANGLE_IN_DEGREE:
        angles.append(angle*180/math.pi)
    else:
        angles.append(angle)
    
    print angle
    
print angles

raw_input()