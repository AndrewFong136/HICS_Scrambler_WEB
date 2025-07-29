const sortData = (data, param, direction = "asc", func) => {
    const sortedData = 
        direction == "asc"
        ? [...data].sort((a, b) => {
            if (a[param] < b[param]) {
                return -1;
            }
            if (a[param] > b[param]) {
                return 1;
            }
            return 0;
        })
        : [...data].sort((a, b) => {
            if (b[param] < a[param]) {
                return -1;
            }
            if(b[param] > a[param]) {
                return 1;
            }
            return 0;
        });
    
    console.log(sortedData);
    func(sortedData);
}