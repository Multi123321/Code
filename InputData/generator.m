function generator(mu, k, sigma,S0)
%GENERATOR Summary of this function goes here
%   Detailed explanation goes here
n = 1000000;           % Anzahl der Knicke/Tage

T = (1:n);          
M = 1;          % Anzahl der Pfade

E = sigma * randn(1,n);
S = S0 * ones(1,n);

for j=2:n
    S(j) = ((S(j-1))^(1-k))*exp(k*mu + E(j-1));
end;


%S = floor(S*10000)/10000;

out = zeros(n, 2);

out(:,1) = 0:200:200*(n-1);

% disp(size(S));
% disp(size(out));

out(:,2) = S;

csvwrite('kurs.csv', out);

% plot (T,S);