def gemspec
  @gemspec ||= begin
    file = File.expand_path('../stata.gemspec', __FILE__)
    eval(File.read(file), binding, file)
  end
end

begin
  require 'rake/gempackagetask'
rescue LoadError
  task(:gem) { $stderr.puts '`gem install rake` to package gems' }
else
  Rake::GemPackageTask.new(gemspec) do |pkg|
    pkg.gem_spec = gemspec
  end
  task :gem => :gemspec
end

desc "validate the gemspec"
task :gemspec do
  gemspec.validate
end

desc "build extension"
task :ext do
  Dir.chdir('ext') do
    system "ruby extconf.rb && make -B"
  end
end

begin
  require 'rspec/core/rake_task'
  RSpec::Core::RakeTask.new(:spec)
rescue LoadError
end

