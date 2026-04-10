spec main:
    show "Testing Error Handling..."
    
    attempt:
        show "Causing error..."
        trigger Error("Something bad happened!")
    trap Error:
        show "Caught Error!"
    always:
        show "Cleanup done."
    done
    
    show "Execution continues."
done

main()
