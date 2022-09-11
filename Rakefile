require_relative "./cmake_utils/Rakefile_common.rb"

desc "setup Eigen3, FMT and ZSTR"
task :install_3rd do
  FileUtils.cd "ThirdParties"
    sh "rake install"
  FileUtils.cd ".."
end

desc "compile for Visual Studio [default year=2017, bits=x64]"
task :build_win, [:year, :bits] do |t, args|

  args.with_defaults( :year => "2017", :bits => "x64" )

  puts "UTILS build on windows".green

  FileUtils.rm_rf   'lib'
  FileUtils.rm_rf   'build'
  FileUtils.mkdir_p 'build'
  FileUtils.cd      'build'

  FileUtils.mkdir_p "../lib/lib"
  FileUtils.mkdir_p "../lib/bin"
  FileUtils.mkdir_p "../lib/bin/"+args.bits
  FileUtils.mkdir_p "../lib/dll"
  FileUtils.mkdir_p "../lib/include"

  cmd_cmake = cmake_generation_command(args.bits,args.year) + cmd_cmake_build()

  puts "run CMAKE for UTILS".yellow
  sh cmd_cmake + ' ..'
  puts "compile with CMAKE for UTILS".yellow
  if COMPILE_DEBUG then
    sh 'cmake --build . --config Debug --target install '+PARALLEL+QUIET
  else
    sh 'cmake --build . --config Release --target install '+PARALLEL+QUIET
  end

  FileUtils.cd '..'
end

task :build_osx_linux_mingw do
  puts "UTILS build (osx/linux/mingw)".green

  FileUtils.rm_rf   'lib'
  FileUtils.rm_rf   'build'
  FileUtils.mkdir_p 'build'
  FileUtils.cd      'build'

  cmd_cmake = "cmake " + cmd_cmake_build()

  puts "run CMAKE for UTILS".yellow
  sh cmd_cmake + ' ..'
  puts "compile with CMAKE for UTILS".yellow
  if COMPILE_DEBUG then
    sh 'cmake --build . --config Debug --target install '+PARALLEL+QUIET
  else
    sh 'cmake --build . --config Release --target install '+PARALLEL+QUIET
  end

  FileUtils.cd '..'
end

desc 'compile for OSX'
task :build_osx => :build_osx_linux_mingw

desc 'compile for LINUX'
task :build_linux => :build_osx_linux_mingw

desc 'compile for LINUX'
task :build_mingw => :build_osx_linux_mingw

task :clean do
  FileUtils.rm_rf 'lib'
end

desc "clean for OSX"
task :clean_osx => :clean

desc "clean for LINUX"
task :clean_linux => :clean

desc "clean for MINGW"
task :clean_mingw => :clean

desc "clean for WINDOWS"
task :clean_win => :clean

task :cppcheck do
  FileUtils.rm_rf   'lib'
  FileUtils.rm_rf   'build'
  FileUtils.mkdir_p 'build'
  FileUtils.cd      'build'
  sh 'cmake -DCMAKE_EXPORT_COMPILE_COMMAND=ON ..'
  sh 'cppcheck --project=compile_commands.json'
end

desc 'pack for OSX/LINUX/MINGW/WINDOWS'
task :cpack do
  FileUtils.cd "build"
  puts "run CPACK for ROOTS".yellow
  sh 'cpack -C CPackConfig.cmake'
  sh 'cpack -C CPackSourceConfig.cmake'
  FileUtils.cd ".."
end
