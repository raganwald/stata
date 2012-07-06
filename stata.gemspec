# encoding: utf-8

Gem::Specification.new do |s|
  s.name     = 'stata'
  s.version  = '0.1.2'
  s.authors  = ['Kevin Branigan']
  s.summary  = 'Read and write support for the Stata binary format'
  s.homepage = 'http://github.com/unspace/stata'
  s.license  = 'MIT'

  s.add_development_dependency('rspec', '~> 2.4')

  s.extensions    = ['ext/extconf.rb']
  s.files         = `git ls-files LICENSE README ext`.split
  s.require_paths = ['ext']
end

