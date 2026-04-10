blueprint Animal:
    has name
    has age
    
    prep (n, age_val):
        own~>name = n
        own~>age = age_val
    done
    
    spec speak:
        show "Animal speaks"
        show own~>name
    done
    
    spec get_age:
        forward own~>age
    done
done

spec main:
    show "--- Testing OOP (Blueprint/Spawn) ---"
    
    show "1. Creating instance"
    firm dog = spawn Animal("Rex", 3)
    
    show "2. Calling method"
    dog~>speak()
    
    show "3. Getting attribute via method"
    firm dog_age = dog~>get_age()  
    show "Dog age:"
    show dog_age
    
    show "--- OOP Verification Complete ---"
done

main()
