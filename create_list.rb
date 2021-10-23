require 'find'

# Axxxyyyの場合./xxxを指定。指定しない場合は全部
path = ARGV[0] ? ARGV[0] : '.'
# .txtはgitignoreしているので.rbファイルに保存
cnt = 0
ary = []
File.open("squence_list.rb", mode = "w"){|f|
  f.write("#{Time.now.strftime("%Y-%m-%d %H:%M:%S")} 現在\n")
  Find.find(path){|item|
    # ./git とか省く
    if /[0-9]{3}/.match?(File.dirname(item)[2..4])
      # ファイルのみ対象
      # ディレクトリ対象だと空に見えても.empty?でfalseになったため
      if File.ftype(item) == "file"
        base = File.basename(item)
        sequence_num = base[0..5]
        if /[0-9]{6}/.match?(sequence_num)
          if !ary.include?(sequence_num)
            cnt += 1
            ary << sequence_num
            f.write("#{cnt} #{sequence_num}\n")
          end
        end
      end
    end
  }
}
