 <script type="text/javascript"> 
          
        function insertString() { 
            let origString = message; 
            let stringToAdd = "#"; 
            
			
			for(let i = 0; i < origString.length(); i++){
				if(i % 2 === 0){
					newString = origString.slice(0, i) 
                    + stringToAdd 
                    + origString.slice(i); 
  
					 
				}
			}				
  
            
        } 
    </script> 