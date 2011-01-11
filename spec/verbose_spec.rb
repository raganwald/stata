require 'spec_helper'

describe 'Stata.verbose' do
  it "is initialized to false" do
    Stata.verbose.should be_false
  end

  describe "when set to true" do
    before :all do
      @rtn = (Stata.verbose = true)
    end

    it "returns its new value" do
      @rtn.should be_true
    end

    it "sets the new value" do
      Stata.verbose.should be_true
    end
  end

  after :all do
    Stata.verbose = false
  end
end

