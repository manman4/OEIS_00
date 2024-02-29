# If n appears so do 2n+2 and 3n+3.
def A(n)
  a = [4]
  i = 0
  while i <= n
    j = a[i]
    p [j, j * 2 + 2] if a.include?(j * 2 + 2)
    p [j, j * 3 + 3] if a.include?(j * 3 + 3)
    a << j * 2 + 2
    a << j * 3 + 3
    a.sort!.uniq!
    i += 1
  end
  a[0..n - 1]
end

n = 10000
ary = A(n)
