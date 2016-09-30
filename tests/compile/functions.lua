i=0
print(i)
function test(p)
    i=p
end

test(1)
print(i)

test(2)
print(i)

n=3
test(n)
print(i)

function test2(q)
    return 4
end

print(test2(3))
