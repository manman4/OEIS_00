require 'fileutils'

# filenames = []
dir_name = "054"
dir = Dir.open(dir_name)
dir.each{|file_name|
  # filenames << file_name
  if file_name[-4..-1] == ".txt" && file_name[0] == "b"
    b_file_name = file_name[1..-5]
    # 6桁＋α
    if b_file_name.size == 6
      # p b_file_name
      path = dir_name + "/" + b_file_name
      # フォルダがなければ作成
      FileUtils.mkdir_p(path) unless FileTest.exist?(path)

      # b_fileの移動
      # 同名のファイルがなければ、フォルダの中に移動
      if !(File.exists?(path + "/" + file_name))
        FileUtils.mv(dir_name + "/" + file_name, path + "/" + file_name)
      end
    end
  end
}
# p filenames