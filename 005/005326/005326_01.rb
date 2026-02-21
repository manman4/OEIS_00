ARGF.each_line do |line|
  next if line.strip.empty?

  n, val = line.split.map(&:to_i)

  next unless n.even?

  root = Integer.sqrt(val)

  unless root * root == val
    raise "Not a perfect square at n=#{n}, value=#{val}"
  end

  puts root
end