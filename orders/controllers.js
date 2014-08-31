/*
 * Bind AngularJs application to HTML code.
 */
var ordersApp = angular.module('ordersApp', []);


/*
 * Directive which fires a function when an element is scrolled.
 */
ordersApp.directive('whenScrolled', function() {
    return {
        restrict: 'A',
        link: function(scope, elm, attr) {
            var raw = elm[0];
            
            elm.bind('scroll', function() {
                if (raw.scrollTop + raw.offsetHeight >= raw.scrollHeight) {
                    scope.$apply(attr.whenScrolled);
                 }
            });
        }
    };
});


/*
 * Controller for the orders model.
 * Orders and the respective map markers and infowindows are stored in arrays.
 * In each array one order is represented by the same index.
 */
ordersApp.controller('OrdersListCtrl', function ($scope, $http, $timeout) {
	$scope.orders = []; // array of order objects
    $scope.markers = []; // map markers representing orders
    $scope.infowindows = []; // marker tooltips displaying orders info on the map
    $scope.ordersQuantities = []; // associative array, order type counter
    $scope.mostOrdered = ''; // most ordered type name
    $scope.showScrollHint = true;
    $scope.ordersDisplayLimit = 30;
    $scope.pollingFrequency = 1000; // in milliseconds

    // map initially centered over Berlin, zoomed to Europe level
    $scope.mapOptions = {
        center: new google.maps.LatLng(52.5167,13.3833),
        zoom: 4
    };
    $scope.map = new google.maps.Map(document.getElementById("map-canvas"), $scope.mapOptions);
    
    // if there are too many map markers side by side,
    // marker clusterer combines them in a single representation
    $scope.mcOptions = {gridSize: 100};
    $scope.markerCluster = new MarkerClusterer($scope.map, $scope.markers, $scope.mcOptions);
    
    /*
     * Only new markers and respective infowindows are appended to the corresponding
     * arrays. With this approach markers and infowindows do not have to be generated
     * anew for all orders fetched with the REST api.
     * 
     * @return {Integer} Number of new orders
     */
    $scope.populateMarkers = function() {
        var i,
            infowindow,
            latLng,
            marker,
            newOrdersCount = 0; // number of new orders fetched with the REST api
        
        // if server has been restarted, recreate the arrays of markers, infowindows and quantities
        if($scope.orders.length < $scope.markers.length) {
        	$scope.markers = [];
        	$scope.infowindows = [];
        	$scope.ordersQuantities = [];
        }
        
        // for each new order fetched with the REST api, create a map marker and infowindow
        // and append them to the corresponding arrays; also increase the counter for the 
        // order type
        for(i = $scope.markers.length; i < $scope.orders.length; i++) {
            latLng = new google.maps.LatLng($scope.orders[i].geo_lat, $scope.orders[i].geo_long);
            marker = new google.maps.Marker({'position' : latLng, 'title' : $scope.orders[i].name});
            $scope.markers.push(marker);
            infowindow = new google.maps.InfoWindow({content: $scope.orders[i].name + ', ' + $scope.orders[i].price + '€'});
            $scope.infowindows.push(infowindow);
            // open the infowindow when marker is clicked
            google.maps.event.addListener(marker, 'click', (function(marker) {
                return function() {
                    infowindow.open($scope.map,marker);
                }
            })(marker));
            // increase the counter for the order type
            $scope.ordersQuantities[$scope.orders[i].name] = ($scope.orders[i].name in $scope.ordersQuantities) ? $scope.ordersQuantities[$scope.orders[i].name] + 1 : 1;
            newOrdersCount += 1;
        }
        $scope.calculateMostOrdered();
        return newOrdersCount;
    }
    
    
    /*
     * Draws markers on the map.
     */
    $scope.displayMarkers = function() {
        var key,
            newMarkersCount = $scope.populateMarkers();
        
        // draw only new markers (in one go); this prevents the map from refreshing
        // each time a single marker would be added
        if(newMarkersCount > 0) {
            $scope.markerCluster.addMarkers($scope.markers.slice(-newMarkersCount));
        }
    }
    
    
    /*
     * Increases the number of orders to be displayed if a user scrolls the list. 
     */
    $scope.ordersScroll = function() {
        $scope.ordersDisplayLimit += 1000;
        $scope.showScrollHint = false;
    }

    
    /*
     * Centers the map and shows a marker and the corresponding infowindow
     * when order clicked on the list.
     */
    $scope.showMarker = function(markerId) {
        $scope.map.setCenter($scope.markers[markerId].getPosition());
        $scope.map.setZoom(14);
        $scope.infowindows[markerId].open($scope.map,$scope.markers[markerId]);
    }
    
    
    /*
     * Scans ordersQuantities and checks which type is the most ordered.
     */
    $scope.calculateMostOrdered = function() {
        var currentQuantity,
            maxName = '',
            maxQuantity = 0,
            name;
        
        for(name in $scope.ordersQuantities) {
            currentQuantity = $scope.ordersQuantities[name];
            if(currentQuantity >= maxQuantity) {
                maxName = name;
                maxQuantity = currentQuantity;
            }
        }
        maxName = (maxName == 'undefined') ? 'No name' : maxName;
        $scope.mostOrdered = maxName;
    }
        
    
    /*
     * Periodically fetch orders from server with REST api. Uses recursion.
     */
    $scope.poll = function() {
    	// asynchronous call to the server; waits the specified time no matter if
    	// the response arrives or not and recalls the function
        $http.get('/orders', {timeout: $scope.pollingFrequency})
        .success(function(data) {
            $scope.orders = angular.fromJson(data);
            $scope.displayMarkers();
        })
        .finally($timeout($scope.poll, $scope.pollingFrequency));
    };
    
    // start polling
    $scope.poll();
         
});