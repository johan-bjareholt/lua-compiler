
-- Test case 4

print "enter a number:"
n = io.read("*number")
for i = 2,n do
  if n%i==0 then
    io.write(i)
    io.write(" is a factor of ")
    print(n)
  end
end

