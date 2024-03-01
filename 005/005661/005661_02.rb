# If n appears so do 2n-2 and 3n-3.
def A(n)
  a = [5]
  i = 0
  while i <= n
    j = a[i]
    p [j, j * 2 - 2] if a.include?(j * 2 - 2)
    p [j, j * 3 - 3] if a.include?(j * 3 - 3)
    # p [a[i] * 2 + 2, a[i] * 3 + 3]
    a << j * 2 - 2
    a << j * 3 - 3
    a.sort!.uniq!
    i += 1
  end
  a[0..n - 1]
end

n = 2000
ary = A(n)
# (1..n).each{|i|
#   j = ary[i - 1]
#   break if j.to_s.size > 1000
#   print i
#   print ' '
#   puts j
# }