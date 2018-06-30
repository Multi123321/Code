#!/usr/bin/octave -qf

mu = 0;
k = 0.00005;
sigma = 0.002;
S0 = 1;

%GENERATOR Summary of this function goes here
%   Detailed explanation goes here
n = 1000000;           % Anzahl der Knicke/Tage

T = (1:n);          
M = 1;          % Anzahl der Pfade

for i=1:8
    if exist(['kurs' num2str(i) '.csv'], 'file') == 2 
        continue;
    end

    disp(['Starting ' num2str(i)]);

    E = sigma * randn(1,n);
    S = S0 * ones(1,n);

    for j=2:n
        S(j) = ((S(j-1))^(1-k))*exp(k*mu + E(j-1));
    end;


    %S = floor(S*10000)/10000;

    out = zeros(n, 2);

    out(:,1) = 0:200:200*(n-1);
    out(:,2) = S;

    csvwrite(['kurs' num2str(i) '.csv'], out);

    disp([num2str(i) ' done']);
end;
% plot (T,S);