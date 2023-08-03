isok(n) = sigma(n)>sigma(n+1) && sigma(n+1)>sigma(n+2) && sigma(n+2)>sigma(n+3) && sigma(n+3)>sigma(n+4) && sigma(n+4) > sigma(n+5);                                                                    
cnt = 1;
for(n=1, 5209399195, if(isok(n), write("/Users/xxx/Desktop/b050945_1.txt", cnt," ", n); cnt++)) 
