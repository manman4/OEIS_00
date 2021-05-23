def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A013587(n)
  ary = [0, 1]
  (2..n).each{|i|
    s = 0
    (1..i - 1).each{|j|
      k = i - j
      s += ary[j] * ary[k] * j ** 2 * k * (k * ncr(3 * i - 4, 3 * j - 2) - j * ncr(3 * i - 4, 3 * j - 1))
    }
    ary << s
  }
  ary[1..-1]
end

p A013587(14)
