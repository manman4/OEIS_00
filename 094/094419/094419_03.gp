\\ a(n) = (1/7) * Sum_{k>=0} k^n * (6/7)^k.
a(n) = (1/7) * sum(k=0, 600, k^n * (6/7)^k)+0.;
for(n=0, 13, print1(a(n),", "));
for(n=0, 13, print1(round(a(n)), ", "));

