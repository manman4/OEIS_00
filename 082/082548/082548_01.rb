require 'prime'

n = 500
p_ary = [0] + Prime.take(n).to_a

a = [0]

a_ary = [1]

(1..n - 1).each{|i|
  p = p_ary[i]
  b = []
  a.each{|j|
    k = j + p

    b << k
    k = j - p

    b << k
  }
  a = b.uniq
  a_ary << a.size
}
a_ary

(1..n).each{|i|
  print i
  print ' '
  puts a_ary[i - 1]
}

=begin
limit = 500;
M = sum(i = 1, limit, prime(i));
v = vector(M); primeSum = 0;
s = 1
forprime (n = 1, prime(limit), count = 1; forstep (i = primeSum, 1, -1, if (v[i], count = count + 1; v[i + n] = 1)); v[n] = 1; print(s," ",count); s+= 1; primeSum = primeSum + n)
=end
