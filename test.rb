#!/usr/bin/ruby

require 'yaml'
require 'Stata'

[
  'test_file_1',
  'test_file_2',
  'test_file_3',
].each do |file|
  
  s1 = Stata.read "#{file}.dta"
  Stata.write "#{file}_resave.dta", s1
  
  s2 = Stata.read "#{file}_resave.dta"
  ['data', 'fields', 'value_labels'].each do |f|
    if (s1[f] != s2[f])
      puts "ERROR '#{file}' #{f} did not read/write the same!"
    end
  end
  puts "\n"
end
