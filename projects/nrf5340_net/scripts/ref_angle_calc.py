import math

ANGLE_IN_DEGREE = 1

positions = [i*30 for i in range(8)]
angles    = {1:[], 2:[]}

print "positions: ", positions

for pos in positions:
    
    a = 100 - pos
    b = math.sqrt(25**2 + 100**2 + a**2)
    c = math.sqrt(25**2 + 100**2)
    
    print "a, b, c = ", a, b, c
    
    # calculate the angle
    angle_1 = math.acos((a**2 + b**2 - c**2)/(2*a*b))

    a = 25
    b = math.sqrt(25**2 + 100**2 + (100-pos)**2)
    c = math.sqrt(100**2 + (100-pos)**2)
    
    angle_2 = math.acos((a**2 + b**2 - c**2)/(2*a*b))
    if ANGLE_IN_DEGREE:
        angles[1].append(angle_1*180/math.pi)
        angles[2].append((math.pi-angle_2)*180/math.pi)
    else:
        angles[1].append(angle_1)
        angles[2].append(math.pi-angle_2)
    
print angles

raw_input()