# 入力ファイルと出力ファイルのパス
input_path = 'b053442.txt'
output_path = 'output.txt'

# ファイルを読み込んで書き換えた内容を保存
File.open(output_path, 'w') do |out_file|
  File.foreach(input_path).with_index do |line, i|
    _, value = line.strip.split
    out_file.puts "#{i} #{value}"
  end
end

puts "変換が完了しました: #{output_path}"