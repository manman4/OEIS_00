\\ a(n) = (1/5) * Sum_{k>=0} k^n * (4/5)^k.
a(n) = (1/5) * sum(k=0, 600, k^n * (4/5)^k)+0.;
for(n=0, 13, print1(a(n),", "));
for(n=0, 13, print1(round(a(n)), ", "));

