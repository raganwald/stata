#!/usr/bin/env ruby -w
#encoding:ASCII-8BIT

$LOAD_PATH << '.'
require 'Stata'

Stata.verbose = true

files = if ARGV.empty?
          Dir.glob("test_file_*.dta")
        else
          ARGV
        end

if files.empty?
  puts "usage: #{$0} file1.dta [file2.dta ...]"
  exit(-1)
end

files.each do |original|
  if original =~ /_resave/
    puts "skipping #{original.inspect}\n\n"
    next
  end

  ext    = File.extname(original)
  base   = original[0..-(ext.length+1)]
  resave = "#{base}_resave#{ext}"
  
  s1 = Stata.read(original)
  Stata.write(resave, s1)
  
  s2 = Stata.read(resave)

  ['data', 'fields', 'value_labels'].each do |f|
    if (s1[f] != s2[f])
      puts "ERROR '#{file}' #{f} did not read/write the same!"
    end
  end

  puts "\n"
end
