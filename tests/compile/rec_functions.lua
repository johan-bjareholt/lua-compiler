function recfunc(i)
    if (i < 10) then
        i = recfunc(i+1)
    end
    return i
end

a=0
print(a)
a = recfunc(a)
print(a)
