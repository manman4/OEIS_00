require 'prime'

def power0(a, n)
  return 1 if n == 0
  k = power0(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

# x > 0
def sigma(x, i)
  sum = 1
  pq = i.prime_division
  pq.each{|a, n| sum *= (power0(a, (n + 1) * x) - 1) / (power0(a, x) - 1)}
  sum
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  s = 0
  (1..n - 2).each{|i|
    (1..n - 2).each{|j|
      (1..n - 2).each{|k|
        s += sigma(1, i) * sigma(1, j) * sigma(1, k) / (i * j * k).to_r if i + j + k == n
      }
    }
  }
  (f(n) * s).to_i / 6
end

n = 1000
(3..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}

# def B(n)
#   s = 0
#   (1..n - 3).each{|i|
#     (1..n - 3).each{|j|
#       (1..n - 3).each{|k|
#         (1..n - 3).each{|l|
#           s += sigma(1, i) * sigma(1, j) * sigma(1, k) * sigma(1, l) / (i * j * k * l).to_r if i + j + k + l == n
#         }
#       }
#     }
#   }
#   (f(n) * s).to_i / 24
# end

# (4..n).each{|i|
#   j = B(i)
#   break if j.to_s.size > 1000
#   print i
#   print ' '
#   puts j
# }