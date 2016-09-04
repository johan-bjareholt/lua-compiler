-- Test case 3
print "enter a number:"
n = io.read("*number")
factorial = 1
x=1
for i = 2,n do
  x = x * i
end
io.write("factorial of ")
io.write(n)
io.write(" is ")
print(x)
