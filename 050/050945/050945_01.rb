require 'prime'

# 5連続のリスト(x, x+1と続くので、実際は6連続しか見つかっていない)
# ary = [
#   20021154, 20021155, 
#   28503474, 28503475, 
#   53728674, 53728675, 
#   224539314, 224539315, 
#   242917674, 242917675, 
#   304178874, 304178875, 
#   341295954, 341295955, 
#   370845474, 370845475, 
#   377692314, 377692315, 
#   443998554, 443998555, 
#   533007474, 533007475, 
#   610484874, 610484875, 
#   709445274, 709445275, 
#   855529674, 855529675, 
#   964885074, 964885075, 
#   978052074, 978052075, 
#   1069943874, 1069943875, 
#   1114988874, 1114988875, 
#   1167961794, 1167961795, 
#   1207157874, 1207157875, 
#   1210844634, 1210844635, 
#   1231385154, 1231385155, 
#   1286159874, 1286159875, 
#   1290484194, 1290484195, 
#   1295496474, 1295496475, 
#   1363997634, 1363997635, 
#   1475995674, 1475995675, 
#   1525799274, 1525799275, 
#   1568987034, 1568987035, 
#   1621294674, 1621294675, 
#   1637510874, 1637510875, 
#   1656637674, 1656637675, 
#   1683276594, 1683276595, 
#   1696970274, 1696970275, 
#   1743817074, 1743817075, 
#   1774447674, 1774447675, 
#   1877477874, 1877477875, 
#   1886159274, 1886159275, 
#   1940213274, 1940213275, 
#   1962943674, 1962943675, 
#   2007240234, 2007240235, 
#   2197149954, 2197149955, 
#   2231550474, 2231550475, 
#   2278951674, 2278951675, 
#   2301293994, 2301293995, 
#   2313185874, 2313185875, 
#   2356429074, 2356429075, 
#   2424758874, 2424758875, 
#   2478951474, 2478951475, 
#   2518591074, 2518591075, 
#   2569942374, 2569942375, 
#   2589692874, 2589692875, 
#   2590663074, 2590663075, 
#   2628140514, 2628140515, 
#   2697135594, 2697135595, 
#   2723996274, 2723996275, 
#   2744536794, 2744536795, 
#   2797509714, 2797509715, 
#   2815888074, 2815888075, 
#   2860933074, 2860933075, 
#   2938410474, 2938410475, 
#   2967959994, 2967959995, 
#   3054806754, 3054806755, 
#   3067058994, 3067058995, 
#   3100572474, 3100572475, 
#   3183455274, 3183455275, 
#   3205908474, 3205908475, 
#   3221939874, 3221939875, 
#   3349220874, 3349220875, 
#   3367238874, 3367238875, 
#   3381292914, 3381292915, 
#   3463454994, 3463454995, 
#   3520391874, 3520391875, 
#   3729236874, 3729236875, 
#   3826697874, 3826697875, 
#   3869580714, 3869580715, 
#   3887959074, 3887959075, 
#   3908499594, 3908499595, 
#   4141292154, 4141292155, 
#   4157147994, 4157147995, 
#   4161832674, 4161832675, 
#   4264535274, 4264535275, 
#   4286156874, 4286156875, 
#   4310661354, 4310661355, 
#   4322913594, 4322913595, 
#   4331201874, 4331201875, 
#   4342012674, 4342012675, 
#   4367237874, 4367237875, 
#   4412883474, 4412883475, 
#   4469940474, 4469940475, 
#   4567958394, 4567958395, 
#   4592462874, 4592462875, 
#   4621291674, 4621291675, 
#   4763994234, 4763994235, 
#   4819129314, 4819129315, 
#   4821291474, 4821291475, 
#   4990660674, 4990660675, 
#   5086156074, 5086156075, 
# ]
ary = [20021154, 20021155, 28503474, 28503475, 53728674, 53728675, 224539314, 224539315, 242917674, 242917675, 304178874, 304178875, 341295954, 341295955, 370845474, 370845475, 377692314, 377692315, 443998554, 443998555, 533007474, 533007475, 610484874, 610484875, 709445274, 709445275, 855529674, 855529675, 964885074, 964885075, 978052074, 978052075, 1069943874, 1069943875, 1114988874, 1114988875, 1167961794, 1167961795, 1207157874, 1207157875, 1210844634, 1210844635, 1231385154, 1231385155, 1286159874, 1286159875, 1290484194, 1290484195, 1295496474, 1295496475, 1363997634, 1363997635, 1475995674, 1475995675, 1525799274, 1525799275, 1568987034, 1568987035, 1621294674, 1621294675, 1637510874, 1637510875, 1656637674, 1656637675, 1683276594, 1683276595, 1696970274, 1696970275, 1743817074, 1743817075, 1774447674, 1774447675, 1877477874, 1877477875, 1886159274, 1886159275, 1940213274, 1940213275, 1962943674, 1962943675, 2007240234, 2007240235, 2197149954, 2197149955, 2231550474, 2231550475, 2278951674, 2278951675, 2301293994, 2301293995, 2313185874, 2313185875, 2356429074, 2356429075, 2424758874, 2424758875, 2478951474, 2478951475, 2518591074, 2518591075, 2569942374, 2569942375, 2589692874, 2589692875, 2590663074, 2590663075, 2628140514, 2628140515, 2697135594, 2697135595, 2723996274, 2723996275, 2744536794, 2744536795, 2797509714, 2797509715, 2815888074, 2815888075, 2860933074, 2860933075, 2938410474, 2938410475, 2967959994, 2967959995, 3054806754, 3054806755, 3067058994, 3067058995, 3100572474, 3100572475, 3183455274, 3183455275, 3205908474, 3205908475, 3221939874, 3221939875, 3349220874, 3349220875, 3367238874, 3367238875, 3381292914, 3381292915, 3463454994, 3463454995, 3520391874, 3520391875, 3729236874, 3729236875, 3826697874, 3826697875, 3869580714, 3869580715, 3887959074, 3887959075, 3908499594, 3908499595, 4141292154, 4141292155, 4157147994, 4157147995, 4161832674, 4161832675, 4264535274, 4264535275, 4286156874, 4286156875, 4310661354, 4310661355, 4322913594, 4322913595, 4331201874, 4331201875, 4342012674, 4342012675, 4367237874, 4367237875, 4412883474, 4412883475, 4469940474, 4469940475, 4567958394, 4567958395, 4592462874, 4592462875, 4621291674, 4621291675, 4763994234, 4763994235, 4819129314, 4819129315, 4821291474, 4821291475, 4990660674, 4990660675, 5086156074, 5086156075, 5156065914, 5156065915, 5209399194, 5209399195]

# All known members of the sequence are multiples of 6 and one less than a multiple of 35.
# ary.each_with_index{|i, j| p [j + 1, i.prime_division]}

# p (1..ary.size - 2).map{|i| ary[i] - ary[i - 1]}
(1..100).each{|i|
  print i
  print ' '
  puts ary[2 * i - 2]
}
